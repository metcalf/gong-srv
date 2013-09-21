#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "mosquitto.h"
#include "config.h"

#define DEBUG 1

Configration conf;
char my_topic[64];

void debug_print(char* format, ...){
    va_list arglist;
    va_start( arglist, format );
    if(DEBUG)
    {
	vprintf(format, arglist);
    }
    va_end( arglist );
}


void on_log(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    debug_print("%s\n", str);
}

void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
    char topic[64];
    if(!result){
	snprintf(topic, 64, "%s/+/button", conf.topic_root);
	debug_print("Subscribing to topic", topic);
	mosquitto_subscribe(mosq, NULL, topic, 2);
    } else {
	fprintf(stderr, "Connect failed\n");
	mosquitto_loop_stop(mosq, 1);
    }
}

void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	debug_print("Subscribed (mid: %d) (qos's: %d", mid);
	for(i=0; i<qos_count; i++){
	    debug_print(" %d", granted_qos[i]);
	}
	debug_print(")\n");
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    char batter_command[128];
    bool *match;

    debug_print("MSG: %s ", message->topic);
    if(message->payloadlen) {
	debug_print("%s\n", message->payload);
    } else {
	debug_print("(null)\n");
    }
    fflush(stdout);

    mosquitto_topic_matches_sub(my_topic, message->topic, match);
    if(*match){
	debug_print("Received a message on my own topic, ignoring.\n");
	return;
    }
    if(strcmp(message->payload, "released") != 0){
	debug_print("Message is not 'released', ignoring.\n");
	return;
    }

    // Bang ze drum!
    snprintf(batter_command, 64, "batter %s", conf.servo_path);
    system(batter_command);
}

int deinit(){
    mosquitto_lib_cleanup();
    free_config();
}

int main (int argc, char *argv[]) {
    int i;
    char id[30];
    struct mosquitto *mosq = NULL;
    int keepalive = 60;
    bool clean_session = true;

    if(load_config())
    {
	fprintf(stderr, "Error loading config from UCI\n");
	return 1;
    }
    // We're going to pass this to a system call so be really careful
    snprintf(my_topic, 64, "%s/%s/button", conf.topic_root, conf.name);
    for(i=0; strlen(conf.servo_path); i++){
	if(!(isalnum(conf.servo_path[i]) || conf.servo_path[i] == ' ')){
	    fprintf(stderr, "Servo path must have only alphanumeric characters and spaces");
	    return 1;
	}
    }

    if(atexit(deinit))
    {
	fprintf(stderr, "Could not register atexit function\n");
	return 1;
    }

    debug_print("Loaded config. Starting mosquitto client.\n");

    mosquitto_lib_init();

    mosq = mosquitto_new(id, clean_session, NULL);
    if(!mosq){
	fprintf(stderr, "Error: Out of memory.\n");
	return 1;
    }

    mosquitto_log_callback_set(mosq, on_log);
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);

    debug_print("Connecting to MQTT broker at %s:%d", conf.host, conf.port);

    if(mosquitto_connect(mosq, conf.host, conf.port, keepalive)){
	fprintf(stderr, "Unable to connect.\n");
	return 1;
    }

    debug_print("Connected to MQTT broker.  Subscribing to topic");

    while(!mosquitto_loop(mosq, -1, -1)){
    }
    mosquitto_destroy(mosq);

    return 0;
}

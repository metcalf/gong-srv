#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <mosquitto.h>

#include "config.h"

#define DEBUG 1
#define KEEPALIVE 60

Configration conf;
char my_topic[64];
int next_message_id = 0;

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
    debug_print("MOSQUITTO LOG: %s\n", str);
}

void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
    char topic[64];
    if(!result){
	snprintf(topic, 64, "%s/+/button", conf.topic_root);
	debug_print("Subscribing to topic '%s'.\n", topic);
	mosquitto_subscribe(mosq, NULL, topic, 2);
    } else {
	fprintf(stderr, "Connect failed\n");
	mosquitto_disconnect(mosq);
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
    int message_id;
    bool match;

    debug_print("MSG: %s ", message->topic);
    if(message->payloadlen) {
	debug_print("%s\n", message->payload);
    } else {
	debug_print("(null)\n");
    }

    mosquitto_topic_matches_sub(my_topic, message->topic, &match);
    if(match){
	debug_print("Received a message on my own topic, ignoring.\n");
	return;
    }

    if(strncmp(message->payload, "released", message->payloadlen) == 0){
	// Bang ze drum!
	snprintf(batter_command, 128, "batter %s", conf.servo_path);
	debug_print("Running batter: %s\n", batter_command);
	system(batter_command);
    } else if(strncmp(message->payload, "ping", message->payloadlen) == 0) {
	debug_print("Got a ping, sending pong.");

	message_id = next_message_id;
	mosquitto_publish(mosq, &message_id, my_topic, 4, "pong", 0, 0);
	next_message_id++;

    } else {
	debug_print("Unrecognized message, ignoring.\n");
    }
}

int deinit(){
    mosquitto_lib_cleanup();
    free_config();
}

int main (int argc, char *argv[]) {
    int i, rc;
    struct mosquitto *mosq = NULL;

    if(load_config())
    {
	fprintf(stderr, "Error loading config from UCI\n");
	return 1;
    }
    snprintf(my_topic, 64, "%s/%s/button", conf.topic_root, conf.name);

    // We're going to pass this to a system call so be really careful
    for(i=0; i < strlen(conf.servo_path); i++){
	if(!(isalnum(conf.servo_path[i]) || conf.servo_path[i] == ' ')){
	    fprintf(stderr, "Servo path must have only alphanumeric characters and spaces\n");
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

    mosq = mosquitto_new(NULL, true, NULL);
    if(!mosq){
	fprintf(stderr, "Out of memory.\n");
	return 1;
    }

    mosquitto_log_callback_set(mosq, on_log);
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);

    debug_print("Connecting to MQTT broker at %s:%d\n", conf.host, conf.port);

    if(mosquitto_connect(mosq, conf.host, conf.port, KEEPALIVE)){
	fprintf(stderr, "Unable to connect.\n");
	return 1;
    }

    debug_print("Connected to MQTT broker. Starting network loop.\n");

    rc = mosquitto_loop_forever(mosq, -1, 1);

    if(rc){
	fprintf(stderr, "Error in network loop: ");
	if(rc == MOSQ_ERR_ERRNO){
	    fprintf(stderr, "%s\n", strerror(errno));
	} else{
	    fprintf(stderr, "%s\n", mosquitto_strerror(rc));
	}
    }

    debug_print("Network loop finished, cleaning up and exiting.\n");
    mosquitto_destroy(mosq);

    return 0;
}

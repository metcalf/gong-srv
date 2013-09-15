#include <stdio.h>
#include <libircclient.h>
#include <libirc_rfcnumeric.h>

#include "config.h"

#define DEBUG

#ifdef DEBUG
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...){}
#endif

Configration conf;

int deinit(){
    free_config();
}

int setup_irc(){
    irc_callbacks_t callbacks;
    irc_ctx_t ctx;
    irc_session_t * s;
    unsigned short port;

    memset (&callbacks, 0, sizeof(callbacks));

    callbacks.event_connect = event_connect;
    callbacks.event_join = event_join;
    callbacks.event_nick = event_nick;
    callbacks.event_numeric = event_numeric;
    callbacks.event_channel = event_channel;
    callbacks.event_privmsg = event_privmsg;

    s = irc_create_session (&callbacks);

    if ( !s )
    {
	printf ("Could not create session\n");
	return 1;
    }

    ctx.nick = conf.nick;
    ctx.channel = conf.channel;

    free(&callbacks);
}

int main (int argc, char *argv[]) {
    if(load_config())
    {
	fprintf(stderr, "Error loading config from UCI\n");
	exit(1);
    }

    if(atexit(deinit))
    {
	fprintf(stderr, "Could not register atexit function\n");
	exit(1);
    }

    printf("Host: %s\nNick: %s\nChannel: %s\nSecret: %s\n ",
	   conf.host, conf.nick, conf.channel, conf.secret);



    exit(0);
}

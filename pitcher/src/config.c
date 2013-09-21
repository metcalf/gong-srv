#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uci.h>

#include "config.h"

Configration conf;

struct uci_context* uci_init()
{
    return uci_alloc_context();
}

void uci_cleanup(struct uci_context *ctx)
{
    uci_free_context(ctx);
}

int load_option(struct uci_context *ctx, char *option, char *val, int num)
{
    struct uci_ptr p;
    char path[64];

    snprintf(path, 64, "pitcher.%s", option);

    if (uci_lookup_ptr (ctx, &p, path, true) != UCI_OK)
    {
	uci_perror (ctx, path);
	return 1;
    }

    if(!(p.flags & UCI_LOOKUP_COMPLETE))
    {
	fprintf(stderr, "Lookup incomplete for %s\n", path);
	return 1;
    }
    else if(strlen(p.o->v.string) > num)
    {
	fprintf(stderr, "Configuration value length of %d is longer than the provided buffer of %d\n",
		strlen(p.o->v.string), num);
	return 1;
    }
    else
    {
	strncpy(val, p.o->v.string, num);
    }

    return 0;
}

int load_string_option(struct uci_context *ctx, char *option, char **val)
{
    *val = malloc(PITCHER_CONF_MAX_LEN * sizeof(char));
    int retval = load_option(ctx, option, *val, PITCHER_CONF_MAX_LEN);
    return retval;
}

int load_long_option(struct uci_context *ctx, char *option, long int *val)
{
    char strval[16];
    int retval;

    retval = load_option(ctx, option, strval, 16);
    if(retval)
	return retval;

    *val = strtol(strval, NULL, 0);

    /*if(!*val || strval[0] == '0')
      {
      fprintf(stderr, "Error converting string to long: %s\n", strval);
      return 1;
      }*/
    return retval;
}

int load_int_option(struct uci_context *ctx, char *option, int *val)
{
    long int longval;

    if(load_long_option(ctx, option, &longval))
	return 1;

    if(longval > INT_MAX || longval < INT_MIN)
    {
	fprintf(stderr, "Error converting long to integer: %l\n", longval);
	return 1;
    }

    *val = (int) longval;

    return 0;
}

int load_config()
{
    int retval = 0;

    struct uci_context *ctx = uci_init();

    retval += load_string_option(ctx, "mqtt.host", &conf.host);
    retval += load_string_option(ctx, "mqtt.name", &conf.name);
    retval += load_string_option(ctx, "mqtt.topic_root", &conf.topic_root);
    retval += load_int_option(ctx, "mqtt.port", &conf.port);

    retval += load_string_option(ctx, "batter.servo_path", &conf.servo_path);

    uci_cleanup(ctx);

    return retval;
}

void free_config()
{
    free(conf.host);
    free(conf.name);
    free(conf.topic_root);
}

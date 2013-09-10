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

    snprintf(path, 64, "batter.main.%s", option);

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

    retval += load_int_option(ctx, "pwr_gpio", &conf.pwr_gpio);
    retval += load_int_option(ctx, "pwm_dev", &conf.pwm_dev);
    retval += load_long_option(ctx, "pwm_period", &conf.pwm_period);
    retval += load_long_option(ctx, "min_duty", &conf.min_duty);
    retval += load_long_option(ctx, "max_duty", &conf.max_duty);
    retval += load_long_option(ctx, "servo_rate", &conf.servo_rate);

    uci_cleanup(ctx);

    return retval;
}

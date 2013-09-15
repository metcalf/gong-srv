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

    snprintf(path, 64, "pitcher.irc.%s", option);

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

int load_config()
{
    int retval = 0;

    struct uci_context *ctx = uci_init();

    retval += load_string_option(ctx, "host", &conf.host);
    printf("Config host %s\n", conf.host);
    retval += load_string_option(ctx, "nick", &conf.nick);
    retval += load_string_option(ctx, "channel", &conf.channel);
    retval += load_string_option(ctx, "secret", &conf.secret);

    uci_cleanup(ctx);

    return retval;
}

void free_config()
{
    free(conf.host);
    free(conf.nick);
    free(conf.channel);
    free(conf.secret);
}

#ifndef PITCHER_CONFIG_H_
#define PITCHER_CONFIG_H_

#define PITCHER_CONF_MAX_LEN 64

typedef struct configuration {
    char *host;
    char *nick;
    char *channel;
    char *secret;
} Configration;

extern Configration conf;

int load_config();
void free_config();

#endif

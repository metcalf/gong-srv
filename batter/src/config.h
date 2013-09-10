#ifndef BATTER_CONFIG_H_
#define BATTER_CONFIG_H_

typedef struct configuration {
    int pwr_gpio;
    int pwm_dev;
    long int pwm_period;
    long int min_duty;
    long int max_duty;
    long int servo_rate; // Time in nanoseconds for servo to move 1% of range
} Configration;

extern Configration conf;

int load_config();

#endif

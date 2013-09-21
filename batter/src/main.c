#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

#define DEBUG 0

Configration conf;

int current_position = -1;
char pwmpath[80];

typedef enum {GO, PAUSE} action_t;

typedef struct {
    action_t type;
    unsigned int value;
} action;

void debug(char* format, ...){
    va_list arglist;
    va_start( arglist, format );
    if(DEBUG)
    {
	vprintf(format, arglist);
    }
    va_end( arglist );
}


int write_to_path(char *path, char *val)
{
    debug("Writing to file: %s %s\n", path, val);
    int fd = open(path, O_WRONLY);
    if(fd < 0)
    {
	fprintf(stderr, "Failed to open fd: %s\n", path);
	return 1;
    }

    if(write(fd, val, strlen(val)) < 0)
    {
	close(fd);
	fprintf(stderr, "Failed to write '%s' to the fd: %s\n", val, path);
	return 1;
    }
    close(fd);
    return 0;
}

int write_pwmfs(char *filename, long unsigned int val)
{
    char path[80];
    char strval[16];

    snprintf(path, 80, "%s%s", pwmpath, filename);
    snprintf(strval, 16, "%lu", val);

    debug("Writing PWM FS %s with %s\n", path, strval);
    if(write_to_path(path, strval))
	return 1;

    return 0;
}

int write_gpio(unsigned int gpio, char *filename, char *val)
{
    char path[80];

    snprintf(path, 80, "/sys/class/gpio/gpio%d/%s", gpio, filename);

    if(write_to_path(path, val))
	return 1;

    return 0;
}

int request_servo()
{
    char path[80];
    char buf[1];

    snprintf(path, 80, "%s%s", pwmpath, "request");

    int fd = open(path, O_RDONLY);
    if(read(fd, buf, 1) < 1)
    {
	fprintf(stderr, "Error requesting the pwm at %s\n", path);
	return 1;
    }
    close(fd);
    return 0;
}

int init_servo(long unsigned int duty)
{
    char gpio_str[3];

    snprintf(pwmpath, 80, "/sys/class/pwm/gpio_pwm.0:%u/", conf.pwm_dev);

    debug("Requesting servo\n");
    if(request_servo())
	return 1;

    debug("Configuring servo\n");
    if(write_pwmfs("period_ns", conf.pwm_period) ||
       write_pwmfs("polarity", 1) ||
       write_pwmfs("duty_ns", duty) ||
       write_pwmfs("run", 1))
	return 1;

    current_position = 0; // Ensure deinit runs

    debug("Powering up\n");
    snprintf(gpio_str, 3, "%u", conf.pwr_gpio);
    if(write_gpio(conf.pwr_gpio, "direction", "out") ||
       write_gpio(conf.pwr_gpio, "value", "1"))
	return 1;

    return 0;
}

void deinit_servo()
{
    if(current_position < 0)
	return; // Never initialized

    char gpio_str[3];
    snprintf(gpio_str, 3, "%u", conf.pwr_gpio);

    write_gpio(conf.pwr_gpio, "value", "0");
    write_pwmfs("run", 0);
}
int move_servo(int position)
{
    unsigned int distance;
    long unsigned int target_duty;

    if(position < 0 || position > 100)
    {
	fprintf(stderr, "Position must be a positive integer 0-100, not %d\n", position);
    }

    // Might be an off by 1 error here but doesn't matter
    if (current_position < 0)
	distance = position > 50 ? position : 100 - position;
    else
	distance = abs(current_position - position);

    // No move
    if(distance == 0)
	return 0;

    target_duty = conf.min_duty + (conf.max_duty - conf.min_duty)*position/100;

    if(current_position < 0)
    {
	if(init_servo(target_duty))
	    return 1;
    }
    else if(write_pwmfs("duty_ns", target_duty))
	return 1;

    // Wait for servo to reach position before continuing
    nanosleep((struct timespec[]){{0, distance * conf.servo_rate}}, NULL);
    current_position = position;

    return 0;
}

int perform_actions(action actions[], int num)
{
    action curr;
    action_t action_type;
    unsigned int action_value;
    int i;

    for(i=0; i < num; i++)
    {
	curr = actions[i];

	if(curr.type == PAUSE)
	{
	    debug("Sleeping for %u:%lu\n", curr.value / 1000, (curr.value % 1000) * 1000000UL);
	    nanosleep((struct timespec[]){{curr.value / 1000, (curr.value % 1000) * 1000000UL }}, NULL);
	}
	else if(curr.type == GO)
	{
	    debug("Moving to position %u\n", curr.value);
	    if(move_servo(curr.value))
		return 1;
	}
	else
	{
	    fprintf(stderr, "Received an invalid action type.\n");
	    return 1;
	}
    }

    return 0;
}

void print_help(char *name)
{
    printf("\nUsage: %s [action] [action] ...\n\n"
	   "    Each action is of the form: [type][value]\n"
	   "    Where type is one of G (Go) or P (Pause)\n"
	   "    For Go actions, the value is an integer 0-100 specifying\n"
	   "    the servo position as a percent of the total range.\n"
	   "    For Pause actions, the value is time in milliseconds to pause.\n\n"
	   "    For example, the following command would go to 10%%, wait 1500ms and go to 60%%:\n"
	   "        %s G10 P1500 G60\n",
	   name, name);
}

int main (int argc, char *argv[])
{
    action *actions;
    action_t action_type;
    unsigned int action_value;
    int i;

    if(load_config())
    {
	fprintf(stderr, "Error loading config from UCI\n");
	exit(1);
    }

    if(atexit(deinit_servo))
    {
	fprintf(stderr, "Could not register atexit function\n");
	exit(1);
    }

    if(argc < 1)
    {
	fprintf(stderr, "Unxpected argc less than 1 (%d)", argc);
	exit(1);
    }
    if(argc == 1)
    {
	print_help(argv[0]);
	exit(1);
    }
    if(argc > 100)
    {
	fprintf(stderr, "Stop screwing around!");
	exit(1);
    }

    actions = malloc(sizeof(action)*(argc-1));

    for(i=0; i < argc-1; i++)
    {
	action_value = atoi(argv[i+1]+1);

	switch(argv[i+1][0])
	{
	case 'G':
	case 'g':
	    action_type = GO;
	    if(action_value < 0 || action_value > 100)
	    {
		fprintf(stderr, "action value for Go must be a positive integer 0-100, not %d\n", action_value);
		print_help(argv[0]);
		exit(1);
	    }
	    break;
	case 'P':
	case 'p':
	    action_type = PAUSE;
	    if(action_value < 0 || action_value > 10000)
	    {
		fprintf(stderr, "action value for Pause must be a positive integer 0-10000, not %d\n", action_value);
		print_help(argv[0]);
		exit(1);
	    }
	    break;
	default:
	    fprintf(stderr, "Invalid action type '%c'.\n\n", argv[i+1][0]);
	    print_help(argv[0]);
	    exit(1);

	}

	actions[i] = (action){ .type = action_type, .value = action_value };
    }

    perform_actions(actions, argc-1);

    exit(0);
}

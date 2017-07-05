#ifndef MODLED_H
#define MODLED_H

#define LARGE_BUFFER 512
#define MAX_BUFFER 1024

#define LED_ROOT_DIR "/sys/class/leds/"
#define LED_BRIGHTNESS "brightness"
#define LED_MAX_BRIGHTNESS "max_brightness"
#define LED_TRIGGER "trigger"

#define DEFAULT_LED_ARGS { 0, 0, 0, 0, 0, 0 }
struct led_args {
	char *brightness;
	char *trigger;
	char *LED;
	int help;
	int show_info;
	int print_list;
};

#endif

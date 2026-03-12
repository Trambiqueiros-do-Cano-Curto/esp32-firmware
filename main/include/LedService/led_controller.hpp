#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

typedef enum { LED_CMD_SET, LED_CMD_TOGGLE } led_cmd_type_t;

typedef struct {
    led_cmd_type_t type;
    bool value;
} led_cmd_t;

void led_controller_init();
void led_controller_handler(void *arg);
void led_controller_set_status(bool on);
void led_controller_toggle_status();

#endif
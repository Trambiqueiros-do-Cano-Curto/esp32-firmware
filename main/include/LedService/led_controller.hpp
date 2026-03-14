#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

namespace controller::led{

typedef enum { LED_CMD_SET } led_cmd_type_t;

typedef struct {
    led_cmd_type_t type;
    bool value;
} led_cmd_t;

void init();
void handler(void *arg);

void set_status(bool value);

}

#endif
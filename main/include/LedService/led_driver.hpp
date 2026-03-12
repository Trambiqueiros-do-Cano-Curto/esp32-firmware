#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "soc/gpio_num.h"
#include <stdbool.h>
#include <stdint.h>

void led_driver_init(gpio_num_t pin);
void led_driver_set(gpio_num_t pin, bool state);
void led_driver_toggle(gpio_num_t pin);

#endif
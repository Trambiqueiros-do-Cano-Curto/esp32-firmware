#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "soc/gpio_num.h"
#include <stdbool.h>
#include <stdint.h>

namespace driver::led {
    void init(gpio_num_t pin);
    void set_gpio(gpio_num_t pin, bool state);
}


#endif
#include "LedService/led_driver.hpp"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"

void driver::led::init(gpio_num_t pin) {
  gpio_config_t config = {.pin_bit_mask = (1ULL << pin),
                          .mode = GPIO_MODE_OUTPUT,
                          .pull_up_en = GPIO_PULLUP_DISABLE,
                          .pull_down_en = GPIO_PULLDOWN_DISABLE,
                          .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&config);
}

void driver::led::set_gpio(gpio_num_t pin, bool state) { gpio_set_level(pin, state); }
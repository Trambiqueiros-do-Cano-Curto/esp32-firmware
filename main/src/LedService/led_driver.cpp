#include "LedService/led_driver.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"

void led_driver_init(gpio_num_t pin) {
    gpio_config_t config = {.pin_bit_mask = (1ULL << pin),
                            .mode = GPIO_MODE_OUTPUT,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&config);
}

void led_driver_set(gpio_num_t pin, bool state) { gpio_set_level(pin, state); }

void led_driver_toggle(gpio_num_t pin) {
    int level = gpio_get_level(pin);

    gpio_set_level(pin, !level);
}
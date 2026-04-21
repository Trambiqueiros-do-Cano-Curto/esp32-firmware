#include "Application/button_service.hpp"

#include "LedService/led_controller.hpp"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"

#include "esp_log.h"

#include "utils.hpp"

#include "Network/network_controller.hpp"

namespace service::button {

constexpr gpio_num_t nBOOT_BUTTON = GPIO_NUM_9;

void init() {
    gpio_config_t config = {.pin_bit_mask = (1ULL << nBOOT_BUTTON),
                            .mode = GPIO_MODE_INPUT,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&config);
}

void buttonReleased() {
    controller::network::ping_neighborhood();
    controller::led::set_status(0);
}

void buttonPressed() {
    controller::led::set_status(1);
}

void handler() {
    static Timer buttonTime;
    static bool lastState = 0;

    if (!buttonTime.hasElapsed(100)) {
        return;
    }

    buttonTime.reset();

    bool state = gpio_get_level(nBOOT_BUTTON);

    if (state == 1 && lastState == 0) {
        buttonPressed();
    }

    if (state == 0 && lastState == 1) {
        buttonReleased();
    }

    lastState = state;
}

} // namespace service::button
#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"

extern "C" {
void app_main(void) {
    led_controller_init();
    application_controller_init();
}
}
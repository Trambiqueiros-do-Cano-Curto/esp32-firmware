#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"

extern "C" {
void app_main(void) {
    controller::led::init();
    controller::application::init();
}
}
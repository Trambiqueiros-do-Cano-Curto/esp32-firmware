#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"

void controller::application::init() {
    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    for (;;) {
        controller::led::set_status(true);
        vTaskDelay(pdMS_TO_TICKS(500));
        controller::led::set_status(false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

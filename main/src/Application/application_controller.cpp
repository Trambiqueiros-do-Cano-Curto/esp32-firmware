#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"

void application_controller_init() {
    xTaskCreate(application_controller_handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void application_controller_handler(void *arg) {
    for (;;) {
        led_controller_toggle_status();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

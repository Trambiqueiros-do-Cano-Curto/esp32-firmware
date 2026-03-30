#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "MqttService/mqtt_controller.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>

void controller::application::init() {
    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    uint32_t simulated_reading = 0;
    char payload_buffer[64];

    for (;;) {
        controller::led::set_status(true);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        simulated_reading++;
        snprintf(payload_buffer, sizeof(payload_buffer), "{\"temperature\": %lu}", 20 + (unsigned long)(simulated_reading % 10));
        
        controller::mqtt::publish("/tcc/cluster1/temperature", payload_buffer);

        controller::led::set_status(false);
        vTaskDelay(pdMS_TO_TICKS(4500));
    }
}
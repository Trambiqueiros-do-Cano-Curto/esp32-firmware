#include "LedService/led_controller.hpp"

#include "Network/network_controller.hpp"

#include "Application/application_controller.hpp"
#include "Application/button_service.hpp"
#include "Application/discover_service.hpp"
#include "Application/nvs_service.hpp"

#include "Network/network_service.hpp"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <cstdint>
#include "MqttService/mqtt_controller.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>

void controller::application::init() {

    service::application::nvs::init();

    controller::led::init();
    controller::network::init();

    service::application::button::init();
    service::application::discover::init();

    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    uint32_t simulated_reading = 0;
    char payload_buffer[64];

    for (;;) {
        service::application::button::handler();
        service::application::discover::handler();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
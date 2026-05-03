#include "LedService/led_controller.hpp"

#include "Network/network_controller.hpp"

#include "Application/application_controller.hpp"
#include "Application/button_service.hpp"
#include "Application/discover_service.hpp"

#include "Network/network_service.hpp"
#include "MqttService/mqtt_controller.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void controller::application::init() {
    controller::led::init();
    controller::network::init();
    controller::mqtt::init();

    service::application::button::init();
    service::application::discover::init();

    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    for (;;) {
        service::application::button::handler();
        service::application::discover::handler();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

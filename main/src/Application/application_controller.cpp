#include "LedService/led_controller.hpp"

#include "Network/network_controller.hpp"

#include "Application/application_controller.hpp"
#include "Application/button_service.hpp"
#include "Application/nvs_service.hpp"

#include "freertos/idf_additions.h"
#include "portmacro.h"

void controller::application::init() {

    service::nvs::init();

    controller::led::init();
    controller::network::init();

    service::button::init();

    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    for (;;) {
        service::button::handler();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

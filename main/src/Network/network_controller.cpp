#include "Network/network_controller.hpp"

#include "Network/esp_now_driver.hpp"
#include "Network/network_service.hpp"
#include "Network/wifi_driver.hpp"

#include "freertos/idf_additions.h"

#include <array>
#include <cstdint>

namespace controller::network {

static QueueHandle_t network_queue;

void init() {
    driver::wifi::init();
    driver::network::esp_now::init();

    network_queue = xQueueCreate(10, sizeof(network_cmd_t));

    xTaskCreate(handler, "network_controller", 2048, NULL, 5, NULL);
}

void handler(void *arg) {
    network_cmd_t cmd;

    for (;;) {
        if (xQueueReceive(network_queue, &cmd, portMAX_DELAY)) {
            switch (cmd.type) {
            case PINGALL:
                service::network::ping_all_devices_connected();
                break;
            }
        }
    }

    service::network::handler();
}

void ping_neighborhood() {
    network_cmd_t cmd = {.type = PINGALL, .value = 0};

    xQueueSend(network_queue, &cmd, portMAX_DELAY);
};

} // namespace controller::network

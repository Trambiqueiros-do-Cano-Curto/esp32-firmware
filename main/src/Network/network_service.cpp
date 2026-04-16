#include "Network/network_service.hpp"
#include "Network/esp_now_driver.hpp"
#include "esp_err.h"
#include "esp_log.h"

#include <array>
#include <cstdint>

namespace service::network {

static constexpr uint8_t MAX_DEVICE_TABLE = 10;

static constexpr uint8_t HEADER_CMD = 1;

static std::array<uint32_t, MAX_DEVICE_TABLE> devices;

void init() {}

void handler() {}

void ping_all_devices_connected() {

    esp_err_t ret;

    std::array<uint8_t, HEADER_CMD> data = {
        static_cast<uint8_t>(RxCommand::PING),
    };

    ret = driver::network::esp_now::send_broadcast(data.data(), data.size());

    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "Succesful!");
    } else {
        ESP_LOGE(__FUNCTION__, "Failed! Error: %u", ret);
    }
}

} // namespace service::network
#include "Network/network_service.hpp"
#include "Network/esp_now_driver.hpp"

#include <array>
#include <cstdint>

namespace service::network {

static constexpr uint8_t MAX_DEVICE_TABLE = 10;

static constexpr uint8_t HEADER_CMD = 1;

static std::array<uint32_t, MAX_DEVICE_TABLE> devices;

void init() {}

void handler() {}

void ping_all_devices_connected() {

    std::array<uint8_t, HEADER_CMD> data = {
        ESP
    }

    driver::network::esp_now::send_broadcast(const uint8_t *data, size_t len)
}

} // namespace service::network
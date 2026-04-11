#include "Network/network_service.hpp"

#include <array>
#include <cstdint>

namespace service::network {

static constexpr uint8_t MAX_DEVICE_TABLE = 10;

static std::array<uint32_t, MAX_DEVICE_TABLE> devices;

void init() {}

void handler() {}

void ping_all_devices_connected() {}

} // namespace service::network
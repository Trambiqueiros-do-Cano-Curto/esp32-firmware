#pragma once

#include <cstdint>
namespace service::network {

enum class RxCommand : uint8_t {
    ACK = 0,
    PING,
};

enum class TxCommand : uint8_t {
    ACK = 0,
    PING,
};

void init();
void handler();

void ping_all_devices_connected();

} // namespace service::network
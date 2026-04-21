#pragma once

#include "Network/network_controller.hpp"
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

void ping_broadcast();
void ping_peer(controller::network::MacAddr dest_mac);

} // namespace service::network
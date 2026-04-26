#pragma once

#include "Network/network_controller.hpp"
#include <cstdint>
namespace service::network {

enum class RxCommand : uint8_t {
    ACK = 0,
    PING,

    SIZE,
};

enum class TxCommand : uint8_t {
    ACK = 0,
    PING,

    SIZE,
};

void init();
void handler();

void ping_broadcast();
void ping_peer(controller::network::MacAddr dest_mac);
void add_esp_peer(controller::network::MacAddr peer_mac, uint8_t peer_channel);

} // namespace service::network
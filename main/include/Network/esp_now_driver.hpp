#pragma once

#include "Network/network_controller.hpp"
#include "Network/network_service.hpp"
#include "esp_err.h"
#include "esp_wifi.h"
#include <cstdint>

namespace driver::network::esp_now {

constexpr uint8_t MAX_SIZE_PACKET = 200;

typedef struct __attribute__((packed)) {
    uint8_t command;
    uint8_t src_mac[6];
    uint8_t dest_mac[6];
} PacketHeader;

typedef struct __attribute__((packed)) {
    PacketHeader header;
    uint8_t data[MAX_SIZE_PACKET - sizeof(PacketHeader)];
} Packet;

void init();

void handler();

void register_rx_callback(void (*func)(Packet),
                          service::network::RxCommand cmd);

esp_err_t send_unicast(const uint8_t *dest_mac, service::network::RxCommand cmd,
                       const uint8_t *data);

esp_err_t send_broadcast(service::network::RxCommand cmd, const uint8_t *data);

esp_err_t add_peer(const uint8_t *mac, uint8_t channel);

} // namespace driver::network::esp_now
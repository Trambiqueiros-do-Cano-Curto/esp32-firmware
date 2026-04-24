#pragma once

#include "Network/network_controller.hpp"
#include "esp_err.h"
#include "esp_wifi.h"
#include <cstdint>

namespace driver::network::esp_now {

constexpr uint8_t MAX_SIZE_PACKET = 200;

typedef struct {
    uint8_t src_mac[6];
    uint8_t dest_mac[6];
    uint8_t command;
} PacketHeader;

typedef struct {
    PacketHeader header;
    uint8_t data[MAX_SIZE_PACKET - sizeof(PacketHeader)];
} Packet;

void init();

esp_err_t send_unicast(const uint8_t *dest_mac, const uint8_t *data,
                       size_t len);
esp_err_t send_broadcast(const uint8_t *data, size_t len);

esp_err_t rx_callback(uint8_t *src_addr, void *data, size_t size,
                      wifi_pkt_rx_ctrl_t *rx_ctrl);

uint8_t *data_macs(uint8_t *option);

} // namespace driver::network::esp_now
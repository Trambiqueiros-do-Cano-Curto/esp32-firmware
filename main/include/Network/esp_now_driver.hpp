#pragma once

#include "esp_err.h"
#include "esp_wifi.h"
#include <cstdint>

namespace driver::network::esp_now {
void init();

esp_err_t send_unicast(const uint8_t *dest_mac, const uint8_t *data,
                       size_t len);
esp_err_t send_broadcast(const uint8_t *data, size_t len);

esp_err_t rx_callback(uint8_t *src_addr, void *data, size_t size,
                      wifi_pkt_rx_ctrl_t *rx_ctrl);

uint8_t *data_macs(uint8_t *option);

} // namespace driver::network::esp_now
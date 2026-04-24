#include "Network/esp_now_driver.hpp"
#include "driver/uart.h"
#include "esp_mac.h"
#include "espnow.h"

#include <array>
#include <cstdint>
#include <esp_now.h>

namespace driver::network::esp_now {

constexpr uint8_t MAX_CALLBACK = UINT8_MAX;

std::array<void (*)(Packet), MAX_CALLBACK> RX_CALLBACK_TABLE;

void registerRxCallback(void *func) {}

void rx_callback(const esp_now_recv_info_t *info, const uint8_t *data,
                 int size) {

    // implementação futura de fila de pacotes
}

void init() {
    uart_config_t uart_config = {.baud_rate = 115200,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                 .rx_flow_ctrl_thresh = 0,
#if SOC_UART_SUPPORT_REF_TICK
                                 .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
                                 .source_clk = UART_SCLK_XTAL,
#endif
                                 .flags = {0, 0}};

    // Existing UART setup...
    ESP_ERROR_CHECK(uart_param_config((uart_port_t)0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin((uart_port_t)0, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install((uart_port_t)0,
                                        8 * ESP_NOW_MAX_DATA_LEN,
                                        8 * ESP_NOW_MAX_DATA_LEN, 0, NULL, 0));

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(rx_callback));
}

esp_err_t add_peer(const uint8_t *peer_mac) {
    esp_now_peer_info_t peer = {};

    peer.channel = 0; // Use current channel
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);

    return esp_now_add_peer(&peer);
}

esp_err_t send_unicast(const uint8_t *dest_mac, const uint8_t *data,
                       size_t len) {

    return esp_now_send(dest_mac, data, len);
}

esp_err_t send_broadcast(const uint8_t *data, size_t len) {

    return esp_now_send(ESPNOW_ADDR_BROADCAST, data, len);
}

} // namespace driver::network::esp_now
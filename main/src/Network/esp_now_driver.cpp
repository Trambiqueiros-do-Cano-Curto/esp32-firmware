#include "Network/esp_now_driver.hpp"
#include "LedService/led_controller.hpp"
#include "Network/network_service.hpp"
#include "driver/uart.h"
#include "esp_mac.h"
#include "esp_wifi_types_generic.h"
#include "espnow.h"
#include "freertos/idf_additions.h"
#include "utils.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <esp_now.h>

namespace driver::network::esp_now {

using service::network::RxCommand;

bool isInitialized = 0;

constexpr uint8_t MAX_CALLBACK = UINT8_MAX;
constexpr uint8_t MAX_BUFFER = sizeof(Packet);
constexpr uint8_t ARRIVED_QUEUE_LENGTH = 10;

std::array<void (*)(Packet), MAX_CALLBACK> RX_CALLBACK_TABLE;

static QueueHandle_t esp_now_arrived_queue;

void handler_arrived();

void register_rx_callback(void (*func)(Packet), RxCommand cmd) {
    CHECK_INITIALIZED();

    if (cmd >= RxCommand::SIZE) {
        ESP_LOGE(__FUNCTION__, "cmd: %u isn't a valid RxCommand", cmd);
        return;
    }
    RX_CALLBACK_TABLE[static_cast<uint8_t>(cmd)] = func;
}

void rx_callback(const esp_now_recv_info_t *info, const uint8_t *data,
                 int size) {
    if (size != sizeof(Packet)) {
        ESP_LOGW(__FUNCTION__,
                 "Packet size is incorrect. size: %i src_addr: " MACSTR, size,
                 MAC2STR(info->src_addr));
    }

    static Packet packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(&packet, data, sizeof(packet));

    xQueueSend(esp_now_arrived_queue, &packet, 0);
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

    esp_now_arrived_queue = xQueueCreate(ARRIVED_QUEUE_LENGTH, sizeof(Packet));

    isInitialized = true;
}

void handler() {
    CHECK_INITIALIZED();
    handler_arrived();
}

esp_err_t add_peer(const uint8_t *mac, uint8_t channel) {
    esp_now_peer_info_t peer = {};

    peer.channel = channel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, mac, ESP_NOW_ETH_ALEN);

    return esp_now_add_peer(&peer);
}

esp_err_t send_unicast(const uint8_t *dest_mac, RxCommand cmd,
                       const uint8_t *data) {
    static Packet packet;
    static uint8_t buffer[sizeof(packet)];

    memset(&packet, 0, sizeof(packet));
    memset(&buffer, 0, sizeof(buffer));

    packet.header.command = static_cast<uint8_t>(cmd);
    memcpy(packet.header.dest_mac, dest_mac, sizeof(packet.header.dest_mac));
    esp_wifi_get_mac(WIFI_IF_STA, packet.header.src_mac);

    memcpy(buffer, &packet, sizeof(packet));

    return esp_now_send(dest_mac, buffer, sizeof(buffer));
}

esp_err_t send_broadcast(RxCommand cmd, const uint8_t *data) {
    return send_unicast(ESPNOW_ADDR_BROADCAST, cmd, data);
}

void handler_arrived() {
    static uint8_t buffer[MAX_BUFFER];
    static Packet packet;

    if (xQueueReceive(esp_now_arrived_queue, buffer, 0) == pdFALSE) {
        return;
    }

    memset(&packet, 0, sizeof(packet));
    memcpy(&packet, buffer, sizeof(packet));

    RxCommand cmd = static_cast<RxCommand>(packet.header.command);
    if (cmd >= RxCommand::SIZE) {

        ESP_LOGW(__FUNCTION__,
                 "cmd: %u isn't a valid RxCommand | src_addr: " MACSTR, cmd,
                 MAC2STR(packet.header.src_mac));
        return;
    }

    if (RX_CALLBACK_TABLE[static_cast<uint8_t>(cmd)] == 0) {
        ESP_LOGW(__FUNCTION__, "cmd: %u isn't registered | src_addr: " MACSTR,
                 cmd, MAC2STR(packet.header.src_mac));
        return;
    }

    RX_CALLBACK_TABLE[static_cast<uint8_t>(cmd)](packet);
}

} // namespace driver::network::esp_now
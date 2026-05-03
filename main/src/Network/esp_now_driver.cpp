#include "Network/esp_now_driver.hpp"
#include "LedService/led_controller.hpp"
#include "Network/network_controller.hpp"
#include "Network/network_service.hpp"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi_types_generic.h"
#include "espnow.h"
#include "freertos/idf_additions.h"
#include "utils.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <esp_now.h>

namespace driver::network::esp_now {

using controller::network::MacAddr;
using service::network::RxCommand;

static bool isInitialized = 0;

static constexpr uint8_t MAX_CALLBACK = UINT8_MAX;
static constexpr uint8_t DATA_SIZE = sizeof(Packet);

static constexpr uint8_t MAX_RETRY = 3;

static constexpr uint8_t ARRIVED_QUEUE_LENGTH = 10;
static constexpr uint8_t DEPARTURE_QUEUE_LENGTH = 10;

static MacAddr MY_MAC;

std::array<void (*)(Packet), MAX_CALLBACK> RX_CALLBACK_TABLE;

static QueueHandle_t esp_now_arrived_queue;
static QueueHandle_t esp_now_departure_queue;

void handler_departure();
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
    memcpy(&packet, data, size);

    xQueueSend(esp_now_arrived_queue, &packet, 0);
}

void send_callback(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    static Packet packet;
    if (info->des_addr == nullptr) {
        ESP_LOGI(__FUNCTION__, "1");
        return;
    }

    if (info->data_len < sizeof(packet)) {
        return;
    };

    if (status == ESP_NOW_SEND_SUCCESS) {
        return;
    } else {
        ESP_LOGW(__FUNCTION__, "Packet transmission failed, dest_mac: " MACSTR,
                 MAC2STR(info->src_addr));
    }
}

void place_packet_departure_queue(Packet packet) {
    xQueueSend(esp_now_departure_queue, &packet, 0);
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

    ESP_ERROR_CHECK(esp_now_register_send_cb(send_callback));

    esp_now_arrived_queue = xQueueCreate(ARRIVED_QUEUE_LENGTH, sizeof(Packet));
    esp_now_departure_queue =
        xQueueCreate(DEPARTURE_QUEUE_LENGTH, sizeof(Packet));

    // Add broadcast peer
    add_peer(ESPNOW_ADDR_BROADCAST, 0);

    // Get my mac.
    esp_wifi_get_mac(WIFI_IF_STA, MY_MAC.data());

    isInitialized = true;
}

void handler() {
    CHECK_INITIALIZED();
    handler_arrived();
    handler_departure();
}

esp_err_t add_peer(const uint8_t *mac, uint8_t channel) {
    esp_now_peer_info_t peer = {};

    if (esp_now_is_peer_exist(mac)) {
        return ESP_ERR_ESPNOW_EXIST;
    }

    peer.channel = channel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, mac, ESP_NOW_ETH_ALEN);

    return esp_now_add_peer(&peer);
}

Packet packer(RxCommand cmd, MacAddr dest_mac, std::span<const uint8_t> data) {
    static Packet packet;

    memset(&packet, 0, sizeof(packet));

    packet.command = static_cast<uint8_t>(cmd);

    esp_wifi_get_mac(WIFI_IF_STA, packet.src_mac);

    memcpy(packet.src_mac, MY_MAC.data(), sizeof(MY_MAC));

    memcpy(packet.dest_mac, &dest_mac, sizeof(dest_mac));

    memcpy(packet.data, data.data(), data.size());

    return packet;
}

void send_msg(RxCommand cmd, MacAddr dest_addr, std::span<const uint8_t> data) {
    Packet packet = packer(cmd, dest_addr, data);

    if (memcmp(packet.dest_mac, MY_MAC.data(), sizeof(MY_MAC))) {
        return;
    };

    place_packet_departure_queue(packet);
}

void send_broadcast(RxCommand cmd, std::span<const uint8_t> data) {

    MacAddr broadcast_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFF};

    Packet packet = packer(cmd, broadcast_addr, data);

    place_packet_departure_queue(packet);
}

void handler_departure() {
    static Packet packet;
    static uint8_t buffer[sizeof(packet)];

    static esp_err_t ret = ESP_OK;

    memset(&packet, 0, sizeof(packet));

    if (xQueueReceive(esp_now_departure_queue, &packet, 0) == pdFALSE) {
        return;
    }

    memcpy(buffer, &packet, sizeof(buffer));

    ret = esp_now_send(packet.dest_mac, buffer, sizeof(buffer));
    if (ret == ESP_OK) {
    } else if (ret == ESP_ERR_ESPNOW_NOT_FOUND) {
        ESP_LOGW(__FUNCTION__, "Peer not found!");
    } else {
        ESP_LOGW(__FUNCTION__, "Failed! %s", esp_err_to_name(ret));
    }
}

void handler_arrived() {
    static Packet packet;

    memset(&packet, 0, sizeof(packet));

    if (xQueueReceive(esp_now_arrived_queue, &packet, 0) == pdFALSE) {
        return;
    }

    RxCommand cmd = static_cast<RxCommand>(packet.command);
    if (cmd >= RxCommand::SIZE) {

        ESP_LOGW(__FUNCTION__,
                 "cmd: %u isn't a valid RxCommand | src_addr: " MACSTR, cmd,
                 MAC2STR(packet.src_mac));
        return;
    }

    if (RX_CALLBACK_TABLE[static_cast<uint8_t>(cmd)] == 0) {
        ESP_LOGW(__FUNCTION__, "cmd: %u isn't registered | src_addr: " MACSTR,
                 cmd, MAC2STR(packet.src_mac));
        return;
    }

    if (cmd != RxCommand::ACK) {
    }

    RX_CALLBACK_TABLE[static_cast<uint8_t>(cmd)](packet);
}

} // namespace driver::network::esp_now
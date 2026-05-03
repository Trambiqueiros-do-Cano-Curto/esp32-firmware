#include "Network/network_service.hpp"
#include "LedService/led_controller.hpp"
#include "Network/esp_now_driver.hpp"
#include "Network/network_controller.hpp"
#include "dhcpserver/dhcpserver_options.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdlib.h>

namespace service::network {

using controller::network::MacAddr;

using driver::network::esp_now::Packet;
using driver::network::esp_now::register_rx_callback;

void ping_received(Packet packet);

void init() {
    register_rx_callback(ping_received, RxCommand::PING);
}

void handler() {
    driver::network::esp_now::handler();
}

// ===============================================
// # Ingoing functions
// ===============================================

void ping_received(Packet packet) {
    ESP_LOGI(__FUNCTION__, "Ping received from " MACSTR,
             MAC2STR(packet.src_mac));

    MacAddr macAddr;
    memcpy(&macAddr, packet.src_mac, sizeof(macAddr));
}

// ===============================================
// # Outgoing functions
// ===============================================

void ping_peer(MacAddr dest_mac) {
    std::array<uint8_t, 1> data = {};

    driver::network::esp_now::send_msg(RxCommand::PING, dest_mac, data);
}

void ping_broadcast() {
    std::array<uint8_t, 1> data = {};

    driver::network::esp_now::send_broadcast(RxCommand::PING, data);
}

void add_esp_peer(MacAddr peer_mac, uint8_t peer_channel) {
    esp_err_t ret;

    ret = driver::network::esp_now::add_peer(peer_mac.data(), peer_channel);

    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "New peer added");
    } else if (ret == ESP_ERR_ESPNOW_EXIST) {
        return;
    } else {
        ESP_LOGE(__FUNCTION__, "Failed! Error: %u", ret);
    }
}

} // namespace service::network
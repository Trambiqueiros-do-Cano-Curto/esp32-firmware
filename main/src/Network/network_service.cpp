#include "Network/network_service.hpp"
#include "LedService/led_controller.hpp"
#include "Network/esp_now_driver.hpp"
#include "Network/network_controller.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdlib.h>

namespace service::network {

using controller::network::MacAddr;

using driver::network::esp_now::Packet;
using driver::network::esp_now::register_rx_callback;

static constexpr uint8_t HEADER_CMD = 1;

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
    static bool led = false;
    ESP_LOGI(__FUNCTION__, "Ping received from " MACSTR,
             MAC2STR(packet.header.src_mac));

    led = !led;
    controller::led::set_status(led);

    MacAddr macaddr;
    memcpy(&macaddr, packet.header.src_mac, sizeof(macaddr));
    controller::network::send_ping_device(macaddr);
}

// ===============================================
// # Outgoing functions
// ===============================================

void ping_peer(MacAddr dest_mac) {
    esp_err_t ret;

    std::array<uint8_t, HEADER_CMD> data = {
        static_cast<uint8_t>(RxCommand::PING),
    };

    ret = driver::network::esp_now::send_unicast(dest_mac.data(),
                                                 RxCommand::PING, data.data());

    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "Succesful!");
    } else {
        ESP_LOGE(__FUNCTION__, "Failed! Error: %u", ret);
    }
}

void ping_broadcast() {
    esp_err_t ret;

    std::array<uint8_t, HEADER_CMD> data = {
        static_cast<uint8_t>(RxCommand::PING),
    };

    ret =
        driver::network::esp_now::send_broadcast(RxCommand::PING, data.data());

    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "Succesful!");
    } else {
        ESP_LOGE(__FUNCTION__, "Failed! Error: %u", ret);
    }
}

void add_esp_peer(MacAddr peer_mac, uint8_t peer_channel) {
    esp_err_t ret;

    ret = driver::network::esp_now::add_peer(peer_mac.data(), peer_channel);

    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "Succesful!");
    } else {
        ESP_LOGE(__FUNCTION__, "Failed! Error: %u", ret);
    }
}

} // namespace service::network
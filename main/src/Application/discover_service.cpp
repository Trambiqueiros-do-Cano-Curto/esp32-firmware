#include "Application/discover_service.hpp"
#include "Network/esp_now_driver.hpp"
#include "Network/network_controller.hpp"
#include "Network/network_service.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "utils.hpp"
#include <array>
#include <cstdint>

namespace service::application::discover {

using controller::network::MacAddr;

constexpr uint8_t MAX_CLUSTER = 2;

static std::array<MacAddr, MAX_CLUSTER> peer_cluster;

void init() {

    peer_cluster[0] = {0x1c, 0xdb, 0xd4, 0xf1, 0x4f, 0x00};
    peer_cluster[1] = {0x1c, 0xdb, 0xd4, 0xf0, 0x2d, 0x2c};
    // peer_cluster[2] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x00};
    // peer_cluster[3] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x00};
    // peer_cluster[4] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x00};
    // peer_cluster[5] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x00};

    for (auto it = peer_cluster.begin(); it != peer_cluster.end(); it++) {
        network::add_esp_peer(*it, 0);
    }
}

void handler() {
    static utils::Timer discover_timer;
    static auto it = peer_cluster.begin();

    if (discover_timer.hasElapsed(1000)) {
        it++;
        if (it == peer_cluster.end()) {
            it = peer_cluster.begin();
        }
        ESP_LOGI(__FUNCTION__, "Send ping to:" MACSTR, MAC2STR(it->data()));
        network::ping_peer(*it);
        discover_timer.reset();
    }
}

void add_cluster_peer() {}

} // namespace service::application::discover
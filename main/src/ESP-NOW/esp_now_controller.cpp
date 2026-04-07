#include "ESP-NOW/esp_now_controller.hpp"
#include "ESP-NOW/esp_now_driver.hpp"

/*
    Rever essa função pois precisamos trocar a lógica para
    get e set, estou fazendo do jeito mais burro porque
    é necessário o algoritmo ficar pronto para implementar isso
*/
uint8_t * controller::esp_now::data_macs(uint8_t *option){
    static uint8_t esp1[6] = {0x1c, 0xdb, 0xd4, 0xc4, 0x42, 0xa8};
    static uint8_t esp2[6] = {0x1c, 0xdb, 0xd4, 0xf0, 0x40, 0x80};
    
    if (option[5] == 0xa8) {
        return esp2;
    } else {
        return esp1;
    }
}

void controller::esp_now::init() {
    espnow_storage_init();
    driver::esp_now::init();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    esp_err_t ret = espnow_init(&espnow_config);
    ESP_ERROR_CHECK(ret);

    espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_DATA, true, controller::esp_now::rx_callback);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA); 

    // P2P set
    esp_now_peer_info_t peer_info = {};
    uint8_t *dest_mac = data_macs(mac); 
    memcpy(peer_info.peer_addr, dest_mac, 6);
    peer_info.channel = 1;
    peer_info.ifidx = WIFI_IF_STA;
    peer_info.encrypt = false;

    if (esp_now_add_peer(&peer_info) != ESP_OK) {
        ESP_LOGE("espnow", "Falha ao adicionar peer unicast");
    }

}

esp_err_t controller::esp_now::send_unicast(const uint8_t *dest_mac, const uint8_t *data, size_t len) {
    espnow_frame_head_t frame_head = {};
    frame_head.retransmit_count = 10; 
    frame_head.broadcast = false;
    
    return espnow_send(ESPNOW_DATA_TYPE_DATA, dest_mac, data, len, &frame_head, portMAX_DELAY);
}

esp_err_t controller::esp_now::send_broadcast(const uint8_t *data, size_t len) {
    espnow_frame_head_t frame_head = {};
    frame_head.retransmit_count = 0; 
    frame_head.broadcast = true;
    
    return espnow_send(ESPNOW_DATA_TYPE_DATA, ESPNOW_ADDR_BROADCAST, data, len, &frame_head, portMAX_DELAY);
}

esp_err_t controller::esp_now::rx_callback(uint8_t *src_addr, void *data, size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl) {
    const char *TAG = "app_main";
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);

    ESP_LOGI(TAG, "Recebido de [" MACSTR "]: %.*s", MAC2STR(src_addr), size, (char *)data);

    // implementação futura de fila de pacotes

    return ESP_OK;
}
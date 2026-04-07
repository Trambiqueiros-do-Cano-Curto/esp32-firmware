#include "WifiService/wifi_driver.hpp"
#include "esp_log.h"
#include <cstring>

// Coloque os dados da sua rede aqui
#define WIFI_SSID "boca de fumo do junin"
#define WIFI_PASS "maconhaesexo"

void driver::wifi::init() {
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    wifi_config_t wifi_config = {}; 
    
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());
    
    esp_err_t ret = esp_wifi_connect();
    if (ret == ESP_OK) {
        ESP_LOGI("wifi_driver", "Tentando conectar na rede: %s", WIFI_SSID);
    } else {
        ESP_LOGE("wifi_driver", "Falha ao iniciar conexão: %s", esp_err_to_name(ret));
    }
}
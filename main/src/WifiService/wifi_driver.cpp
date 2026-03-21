#include "WifiService/wifi_driver.hpp"
#include "MqttService/mqtt_controller.hpp"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <string.h>

static const char* TAG = "WIFI_DRIVER";

// O "Ouvinte" que monitora o que acontece com a rede
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Rádio Wi-Fi ligou. Vamos mandar conectar!
        ESP_LOGI(TAG, "Wi-Fi iniciado. Tentando conectar ao AP...");
        esp_wifi_connect();
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Caiu a conexão!
        ESP_LOGW(TAG, "Desconectado do Wi-Fi. Tentando reconectar...");
        
        // Avisa o MQTT para PARAR de tentar enviar e trancar a fila
        controller::mqtt::set_wifi_status(false); 
        
        esp_wifi_connect(); // Tenta reconectar infinitamente
        
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Conectou e pegou IP!
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conectado com sucesso! IP recebido: " IPSTR, IP2STR(&event->ip_info.ip));
        
        // Avisa o MQTT para DESTRAVAR a fila e enviar os dados
        controller::mqtt::set_wifi_status(true); 
    }
}

void driver::wifi::init() {
    // 1. Inicializa a pilha de rede TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 2. Configurações padrão do Wi-Fi (Base do seu colega)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Registra os ouvintes para os eventos de Wi-Fi e IP
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // 4. Puxa as credenciais do menuidf.py menuconfig (Kconfig.projbuild)
    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD);

    // 5. Aplica as configurações e liga o rádio (Base do seu colega)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}
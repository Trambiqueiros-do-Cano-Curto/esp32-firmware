#include "WifiService/wifi_driver.hpp"
#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "MqttService/mqtt_controller.hpp"
#include "nvs_flash.h"

extern "C" {
void app_main(void) {
    // --- INICIALIZAÇÃO DO NVS ---
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ----------------------------

    controller::led::init();
    
    // Inicia o Wi-Fi
    driver::wifi::init(); 
    
    // Inicia o MQTT
    controller::mqtt::init(); 
    
    // Inicia a aplicação
    controller::application::init();
}
}
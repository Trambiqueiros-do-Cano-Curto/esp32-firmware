#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "WifiService/wifi_driver.hpp"
#include "ESP-NOW/esp_now_controller.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include <cstring>

// Biblioteca necessária para iniciar a memória Flash
#include "nvs_flash.h" 

extern "C" {
void app_main(void) {
    // 1. Inicializa o NVS (Non-Volatile Storage) - OBRIGATÓRIO PARA O WI-FI
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Inicializa dependências globais de comunicação
    driver::wifi::init();
    controller::esp_now::init();

    // 3. Inicializa o resto do sistema
    controller::led::init();
    controller::application::init();

    // testes

    // const char* msg = "Cruzeiro 6 x 1 Frangas";
    // uint8_t dest[6] = {0x1c, 0xdb, 0xd4, 0xc4, 0x42, 0xa8};
    // for(;;) {
    //     controller::esp_now::send_broadcast((const uint8_t*)msg, strlen(msg));
    //     vTaskDelay(pdMS_TO_TICKS(3000));
    // }
}
}
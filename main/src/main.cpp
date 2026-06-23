#include "Application/application_controller.hpp"
#include "esp_pm.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

extern "C" {
void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if CONFIG_PM_ENABLE
    // Light sleep automatico: a CPU dorme nos buracos do loop e nas janelas
    // em que o radio esta desligado (MEMBER). O radio do LEADER fica cheio
    // (PS_NONE), entao ele nao dorme o modem.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 40,
        .light_sleep_enable = true,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif

    controller::application::init();
}
}

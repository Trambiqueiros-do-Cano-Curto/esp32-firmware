#include "Application/nvs_service.hpp"

#include "esp_err.h"
#include "nvs_flash.h"

namespace service::application::nvs {

void init() {
    esp_err_t err = nvs_flash_init();

    ESP_ERROR_CHECK(err);
}

} // namespace service::application::nvs
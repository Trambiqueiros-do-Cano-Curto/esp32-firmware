#ifndef ESP_NOW_DRIVER_H
#define ESP_NOW_DRIVER_H

#include "driver/uart.h"
#include "espnow.h"

namespace driver::esp_now {
    void init();
}

#endif
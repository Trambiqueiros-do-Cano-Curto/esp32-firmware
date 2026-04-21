#pragma once

#include "esp_timer.h"
#include <chrono>

struct Timer {
    unsigned long last = 0;

    bool hasElapsed(unsigned long interval) {
        unsigned long now = esp_timer_get_time() / 1000;
        if (now - last >= interval) {
            last = now; // auto reset
            return true;
        }
        return false;
    }

    void reset() {
        last = esp_timer_get_time() / 1000;
    }
};
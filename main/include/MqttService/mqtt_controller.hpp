#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdint.h>

namespace controller::mqtt {

typedef struct {
    char topic[64];
    char payload[256];
} mqtt_msg_t;

extern const int WIFI_CONNECTED_BIT;
extern EventGroupHandle_t mqtt_event_group;

void init();
void handler(void *arg);

void publish(const char *topic, const char *payload);

void set_wifi_status(bool connected);

}

#endif
#include "Application/application_controller.hpp"
#include "LedService/led_controller.hpp"
#include "MqttService/mqtt_controller.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio> // Para o snprintf

void controller::application::init() {
    xTaskCreate(controller::application::handler,
                "application_controller_handler", 4096, NULL, 5, NULL);
}

void controller::application::handler(void *arg) {
    int contador_fake = 0;
    char payload_buffer[64];

    for (;;) {
        controller::led::set_status(true);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Simula uma leitura de sensor
        contador_fake++;
        snprintf(payload_buffer, sizeof(payload_buffer), "{\"temperatura\": %d}", 20 + (contador_fake % 10));
        
        // Manda o dado para a fila do MQTT
        controller::mqtt::publish("/tcc/cluster1/temperatura", payload_buffer);

        controller::led::set_status(false);
        vTaskDelay(pdMS_TO_TICKS(4500)); // Envia a cada 5 segundos
    }
}
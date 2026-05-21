#include "MqttService/mqtt_controller.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "string.h"
#include "sdkconfig.h"
#include "esp_mac.h"

static const char *TAG = "MQTT_CONTROLLER";

static QueueHandle_t mqtt_queue;
static EventGroupHandle_t network_event_group;
static esp_mqtt_client_handle_t mqtt_client;

static const EventBits_t WIFI_CONNECTED_BIT = BIT0;
static const EventBits_t MQTT_CONNECTED_BIT = BIT1; 

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED: {
            xEventGroupSetBits(network_event_group, MQTT_CONNECTED_BIT); 
            ESP_LOGI(TAG, "Conexao estabelecida com o broker MQTT.");

            const char *topico = "v1/dispositivo/teste";
            const char *payload = "{\"temperatura\": 25.5, \"umidade\": 60}";
            controller::mqtt::publish(topico, payload);

            break;
        }
        case MQTT_EVENT_DISCONNECTED:
            xEventGroupClearBits(network_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGW(TAG, "Desconectado do broker MQTT.");
            break;
        case MQTT_EVENT_PUBLISHED: {
            ESP_LOGI(TAG, "Mensagem publicada. ID: %d", event->msg_id);
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Erro interno reportado pela biblioteca MQTT.");
            break;
        default:
            break;
    }
}

void controller::mqtt::init(void) {
    mqtt_queue = xQueueCreate(10, sizeof(mqtt_msg_t));
    network_event_group = xEventGroupCreate();

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    static char client_id[30];
    snprintf(client_id, sizeof(client_id), "esp32_%02x%02x%02x%02x%02x%02x", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = CONFIG_MQTT_BROKER_URI;
    mqtt_cfg.credentials.username = CONFIG_MQTT_BROKER_USERNAME;
    mqtt_cfg.credentials.authentication.password = CONFIG_MQTT_BROKER_PASSWORD;
    mqtt_cfg.credentials.client_id = client_id;
    mqtt_cfg.network.timeout_ms = 10000;
    mqtt_cfg.session.keepalive = 15;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    xTaskCreate(controller::mqtt::handler, "mqtt_controller", 4096, NULL, 5, NULL);
}

void controller::mqtt::handler(void *arg) {
    mqtt_msg_t msg;

    for (;;) {
        if (xQueueReceive(mqtt_queue, &msg, portMAX_DELAY)) {
            xEventGroupWaitBits(network_event_group,
                                MQTT_CONNECTED_BIT, 
                                pdFALSE,
                                pdTRUE,
                                portMAX_DELAY);

            int32_t msg_id = esp_mqtt_client_publish(mqtt_client, msg.topic, msg.payload, 0, 1, 0);

            if (msg_id >= 0) {
                ESP_LOGI(TAG, "Payload publicado: [%s] no topico [%s]", msg.payload, msg.topic);
            } else {
                ESP_LOGE(TAG, "Falha ao alocar mensagem no buffer de transmissao MQTT.");
            }
        }
    }
}

void controller::mqtt::publish(const char* topic, const char* payload) {
    mqtt_msg_t msg;

    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';

    strncpy(msg.payload, payload, sizeof(msg.payload) - 1);
    msg.payload[sizeof(msg.payload) - 1] = '\0';

    if (xQueueSend(mqtt_queue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Fila MQTT cheia, mensagem descartada.");
    }
}

void controller::mqtt::set_wifi_status(bool connected) {
    if (connected) {
        xEventGroupSetBits(network_event_group, WIFI_CONNECTED_BIT);
    } else {
        xEventGroupClearBits(network_event_group, WIFI_CONNECTED_BIT);
    }
}
#include "MqttService/mqtt_controller.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "MQTT_CONTROLLER";

// Variáveis estáticas do controlador
static QueueHandle_t mqtt_queue;
static EventGroupHandle_t network_event_group;
static esp_mqtt_client_handle_t mqtt_client;

// Bit que representa o estado de ligação do Wi-Fi
#define WIFI_CONNECTED_BIT (1 << 0)

// Callback para lidar com eventos internos do próprio MQTT (conexão, desconexão, etc)
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado ao Broker com sucesso!");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT Desconectado do Broker!");
            break;
        case MQTT_EVENT_PUBLISHED: {
            // As chaves aqui resolvem o "warning: unused variable" limitando o escopo
            esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
            ESP_LOGI(TAG, "Mensagem despachada. ID da mensagem: %d", event->msg_id);
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Ocorreu um erro na biblioteca MQTT");
            break;
        default:
            break;
    }
}

void controller::mqtt::init(void) {
    // 1. Cria a fila para armazenar as mensagens dos sensores
    mqtt_queue = xQueueCreate(10, sizeof(mqtt_msg_t));

    // 2. Cria o Event Group que funcionará como "semáforo" para o estado do Wi-Fi
    network_event_group = xEventGroupCreate();

    // 3. Configuração do Cliente MQTT
    esp_mqtt_client_config_t mqtt_cfg = {};
    // Lembre-se de mudar este IP para o IP da sua máquina/servidor local depois!
    mqtt_cfg.broker.address.uri = "mqtt://192.168.15.18:1883"; 
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // 4. Inicia a Task que vai processar a fila em segundo plano
    xTaskCreate(controller::mqtt::handler, "mqtt_controller", 4096, NULL, 5, NULL);
}

void controller::mqtt::handler(void *arg) {
    mqtt_msg_t msg;

    for (;;) {
        // A Task dorme aqui até que alguém (ex: o application_controller) coloque uma mensagem na fila
        if (xQueueReceive(mqtt_queue, &msg, portMAX_DELAY)) {
            
            ESP_LOGI(TAG, "Mensagem recebida na fila. A verificar conectividade Wi-Fi...");
            
            // O Segredo da operação: Espera bloqueante pela rede Wi-Fi.
            // Se o Wi-Fi não estiver ligado, a task fica parada aqui sem gastar CPU
            // até que o wifi_driver chame 'set_wifi_status(true)'
            xEventGroupWaitBits(network_event_group, 
                                WIFI_CONNECTED_BIT, 
                                pdFALSE,  // Não apaga o bit (o wifi continua ligado após ler)
                                pdTRUE,   // Espera por este bit específico
                                portMAX_DELAY); 

            // Se o código chegou aqui, o Wi-Fi está ligado. Podemos publicar!
            // QoS 1 garante que a mensagem será entregue pelo menos uma vez
            int msg_id = esp_mqtt_client_publish(mqtt_client, msg.topic, msg.payload, 0, 1, 0);
            
            if (msg_id >= 0) {
                ESP_LOGI(TAG, "A publicar payload [%s] no tópico [%s]", msg.payload, msg.topic);
            } else {
                ESP_LOGE(TAG, "Falha ao colocar a mensagem no buffer de envio do MQTT");
            }
        }
    }
}

void controller::mqtt::publish(const char* topic, const char* payload) {
    mqtt_msg_t msg;
    
    // Copia os dados de forma segura para a estrutura para não estourar a memória
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';
    
    strncpy(msg.payload, payload, sizeof(msg.payload) - 1);
    msg.payload[sizeof(msg.payload) - 1] = '\0';

    // Envia a estrutura para a fila
    xQueueSend(mqtt_queue, &msg, portMAX_DELAY);
}

// AQUI ESTÁ A FUNÇÃO QUE DEU O ERRO DE LINKER! Ela controla o "semáforo" do Wi-Fi.
void controller::mqtt::set_wifi_status(bool connected) {
    if (connected) {
        xEventGroupSetBits(network_event_group, WIFI_CONNECTED_BIT);
    } else {
        xEventGroupClearBits(network_event_group, WIFI_CONNECTED_BIT);
    }
}
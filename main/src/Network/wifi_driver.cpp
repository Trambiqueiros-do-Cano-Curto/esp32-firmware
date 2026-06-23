#include "Network/wifi_driver.hpp"
#include "MqttService/mqtt_controller.hpp"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "sdkconfig.h"
#include <string.h>

static const char *TAG = "WIFI_DRIVER";

// Workaround de RF do ESP32-C3 SuperMini: potencia alta na antena mal casada
// distorce o TX. Unidade = 0.25 dBm. 34 = 8.5 dBm. Aplicado apos start.
static constexpr int8_t MAX_TX_POWER = 34;

#if CONFIG_MEMBER_POWER_SAVE
static_assert(CONFIG_MEMBER_WAKE_WINDOW_MS > 0 &&
                  CONFIG_MEMBER_WAKE_WINDOW_MS <= CONFIG_MEMBER_WAKE_INTERVAL_MS,
              "MEMBER_WAKE_WINDOW_MS deve satisfazer 0 < W <= "
              "MEMBER_WAKE_INTERVAL_MS");
#endif

// Backoff de reconexao do STA — usado apenas pelo LEADER (uplink MQTT).
static const uint32_t BACKOFF_MS[] = {500, 1000, 2000, 3000, 5000};
static constexpr size_t BACKOFF_LEN =
    sizeof(BACKOFF_MS) / sizeof(BACKOFF_MS[0]);
static size_t backoff_idx = 0;
static TimerHandle_t reconnect_timer = nullptr;

// true somente quando o no deve manter a associacao STA (papel LEADER).
static bool should_be_connected = false;

static void reconnect_timer_cb(TimerHandle_t) {
    if (should_be_connected) {
        esp_wifi_connect();
    }
}

static void schedule_reconnect() {
    if (reconnect_timer == nullptr) {
        return;
    }
    uint32_t delay_ms = BACKOFF_MS[backoff_idx];
    if (backoff_idx + 1 < BACKOFF_LEN) {
        backoff_idx++;
    }
    ESP_LOGI(TAG, "Proxima tentativa em %u ms (idx=%u)", (unsigned)delay_ms,
             (unsigned)backoff_idx);
    xTimerStop(reconnect_timer, 0);
    xTimerChangePeriod(reconnect_timer, pdMS_TO_TICKS(delay_ms), 0);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG,
                 "Interface STA iniciada (aguardando papel para associar).");
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d =
            (wifi_event_sta_disconnected_t *)event_data;
        controller::mqtt::set_wifi_status(false);
        if (should_be_connected) {
            ESP_LOGW(TAG,
                     "Conexao Wi-Fi perdida (reason=%u, rssi=%d). Reconectando...",
                     d->reason, d->rssi);
            schedule_reconnect();
        } else {
            ESP_LOGI(TAG, "STA desassociado (MEMBER) — sem reconexao.");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Conexao estabelecida. IP=" IPSTR,
                 IP2STR(&event->ip_info.ip));
        backoff_idx = 0;
        controller::mqtt::set_wifi_status(true);
    }
}

void driver::wifi::init() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL,
        &instance_got_ip));

    reconnect_timer = xTimerCreate("wifi_reconnect", pdMS_TO_TICKS(1000),
                                   pdFALSE, nullptr, reconnect_timer_cb);

    wifi_config_t sta_config = {};
    strcpy((char *)sta_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy((char *)sta_config.sta.password, CONFIG_WIFI_PASSWORD);
    sta_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    sta_config.sta.pmf_cfg.capable = true;
    sta_config.sta.pmf_cfg.required = false;
    sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    sta_config.sta.scan_method = WIFI_FAST_SCAN;
    sta_config.sta.channel = CONFIG_NETWORK_FIXED_CHANNEL;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    // Boot/UNDECIDED: radio cheio p/ descoberta e eleicao rapidas.
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    // STA nao-associado nao tem canal: pina no canal fixo do cluster para o
    // ESP-NOW. Ao associar como LEADER, o STA assume o canal do AP — que DEVE
    // ser o mesmo CONFIG_NETWORK_FIXED_CHANNEL.
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_NETWORK_FIXED_CHANNEL,
                                         WIFI_SECOND_CHAN_NONE));

    // Workaround de RF: deve ser chamado APOS esp_wifi_start().
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(MAX_TX_POWER));

    ESP_LOGI(TAG,
             "Wi-Fi STA iniciado (canal %d). Radio cheio ate definir papel.",
             CONFIG_NETWORK_FIXED_CHANNEL);
}

void driver::wifi::exit_low_power() {
    // LEADER/boot-LEADER: radio continuo + associa STA p/ uplink MQTT.
    esp_now_set_wake_window(65535); // RX continuo p/ ESP-NOW
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    if (!should_be_connected) {
        should_be_connected = true;
        backoff_idx = 0;
        ESP_LOGI(TAG, "LEADER: radio cheio, associando ao AP (uplink MQTT)...");
        esp_wifi_connect();
    }
}

void driver::wifi::enter_low_power() {
    // MEMBER: derruba a associacao (sem uplink).
    if (should_be_connected) {
        should_be_connected = false;
        if (reconnect_timer != nullptr) {
            xTimerStop(reconnect_timer, 0);
        }
        esp_wifi_disconnect();
        controller::mqtt::set_wifi_status(false);
    }
#if CONFIG_MEMBER_POWER_SAVE
    // Duty-cycle do radio para o ESP-NOW: acorda WAKE_WINDOW a cada
    // WAKE_INTERVAL. TX do proprio membro continua a qualquer hora.
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(
        CONFIG_MEMBER_WAKE_INTERVAL_MS));
    ESP_ERROR_CHECK(esp_now_set_wake_window(CONFIG_MEMBER_WAKE_WINDOW_MS));
    ESP_LOGI(TAG, "MEMBER low-power: wake %d/%d ms (radio duty-cycle).",
             CONFIG_MEMBER_WAKE_WINDOW_MS, CONFIG_MEMBER_WAKE_INTERVAL_MS);
#else
    // Economia desligada (ensaio de controle): radio cheio, so desassociado.
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    esp_now_set_wake_window(65535);
    ESP_LOGI(TAG, "MEMBER (power-save OFF): radio cheio, STA desassociado.");
#endif
}

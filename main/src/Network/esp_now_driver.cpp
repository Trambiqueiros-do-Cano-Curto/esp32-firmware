#include "Network/esp_now_driver.hpp"
#include "driver/uart.h"
#include "esp_mac.h"
#include "espnow.h"

#include <esp_now.h>

namespace driver::network::esp_now {

/*
    Rever essa função pois precisamos trocar a lógica para
    get e set, estou fazendo do jeito mais burro porque
    é necessário o algoritmo ficar pronto para implementar isso
*/
uint8_t *data_macs(uint8_t *option) {
    static uint8_t esp1[6] = {0x1c, 0xdb, 0xd4, 0xc4, 0x42, 0xa8};
    static uint8_t esp2[6] = {0x1c, 0xdb, 0xd4, 0xf0, 0x40, 0x80};

    if (option[5] == 0xa8) {
        return esp2;
    } else {
        return esp1;
    }
}

void init() {
    uart_config_t uart_config = {.baud_rate = 115200,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                 .rx_flow_ctrl_thresh = 0,
#if SOC_UART_SUPPORT_REF_TICK
                                 .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
                                 .source_clk = UART_SCLK_XTAL,
#endif
                                 .flags = {0, 0}};

    ESP_ERROR_CHECK(uart_param_config((uart_port_t)0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin((uart_port_t)0, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install((uart_port_t)0,
                                        8 * ESP_NOW_MAX_DATA_LEN,
                                        8 * ESP_NOW_MAX_DATA_LEN, 0, NULL, 0));
}

esp_err_t send_unicast(const uint8_t *dest_mac, const uint8_t *data,
                       size_t len) {

    return esp_now_send(dest_mac, data, len);
}

esp_err_t send_broadcast(const uint8_t *data, size_t len) {

    return esp_now_send(ESPNOW_ADDR_BROADCAST, data, len);
}

esp_err_t rx_callback(uint8_t *src_addr, void *data, size_t size,
                      wifi_pkt_rx_ctrl_t *rx_ctrl) {
    const char *TAG = "app_main";
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);

    ESP_LOGI(TAG, "Recebido de [" MACSTR "]: %.*s", MAC2STR(src_addr), size,
             (char *)data);

    // implementação futura de fila de pacotes

    return ESP_OK;
}

} // namespace driver::network::esp_now
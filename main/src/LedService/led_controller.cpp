#include "LedService/led_controller.hpp"
#include "LedService/led_driver.hpp"
#include "freertos/idf_additions.h"
#include <cwchar>

static auto STATUS_LED_PIN = GPIO_NUM_8;

static QueueHandle_t led_queue;

void led_controller_init(void) {
    led_driver_init(STATUS_LED_PIN);

    led_queue = xQueueCreate(10, sizeof(led_cmd_t));

    xTaskCreate(led_controller_handler, "led_controller", 2048, NULL, 5, NULL);
}

void led_controller_handler(void *arg) {
    led_cmd_t cmd;

    for (;;) {
        if (xQueueReceive(led_queue, &cmd, portMAX_DELAY)) {
            switch (cmd.type) {
            case LED_CMD_SET:
                led_driver_set(STATUS_LED_PIN, cmd.value);
                break;

            case LED_CMD_TOGGLE:
                led_driver_toggle(STATUS_LED_PIN);
                break;
            }
        }
    }
}

void led_controller_set_status(bool on) {
    led_cmd_t cmd = {.type = LED_CMD_SET, .value = on};

    xQueueSend(led_queue, &cmd, portMAX_DELAY);
}

void led_controller_toggle_status() {
    led_cmd_t cmd = {.type = LED_CMD_TOGGLE};

    xQueueSend(led_queue, &cmd, portMAX_DELAY);
}
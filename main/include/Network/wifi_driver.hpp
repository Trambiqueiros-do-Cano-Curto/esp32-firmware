#pragma once

#include "esp_wifi.h"

namespace driver::wifi {

// Sobe o stack em WIFI_MODE_STA (sem SoftAP), STA desassociado e pinado no
// canal fixo do cluster para o ESP-NOW. Boot fica com radio cheio (PS_NONE).
void init();

// LEADER/boot-LEADER: radio continuo (PS_NONE) e associa o STA ao AP para o
// uplink MQTT. Idempotente.
void exit_low_power();

// MEMBER: derruba a associacao STA e coloca o radio em duty-cycle
// (PS_MIN_MODEM + ESP-NOW wake-window). Com CONFIG_MEMBER_POWER_SAVE=n,
// apenas desassocia e mantem o radio cheio (ensaio de controle). Idempotente.
void enter_low_power();

}

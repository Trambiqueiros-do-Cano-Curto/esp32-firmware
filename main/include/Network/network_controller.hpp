#pragma once

namespace controller::network {

#define MAX_MSG_ESP_NOW 50u

enum cmd_type {
    PINGALL,
};

typedef struct {
    cmd_type type;
    void *value;
} network_cmd_t;

void init();
void handler(void *arg);

} // namespace controller::network
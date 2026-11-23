#pragma once

#include <stdbool.h>

void tcp_server_task(void *pvParameters);
void tcp_server_request_stop(void);
bool tcp_server_start(void);

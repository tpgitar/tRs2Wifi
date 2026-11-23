#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t configure_led(void);
void task_Led_500ms(void *pvParameters);
void setLedTcpConnectionActive(bool active);
void setLedGreen(void);
void setLedBlue(void);
void setLedRed(void);

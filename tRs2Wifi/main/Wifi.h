#pragma once

#include "esp_err.h"
#include <stdbool.h>

esp_err_t initWifi(void);
bool wifiIsConnected(void);

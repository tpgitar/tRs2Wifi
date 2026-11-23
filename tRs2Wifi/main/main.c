#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Led.h"
#include "Wifi.h"
#include "tcpServer.h"
#include "uart.h"

static const char *TAG = "app";

void app_main(void)
{
    esp_err_t err = configure_led();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED initialization failed: %s", esp_err_to_name(err));
        return;
    }

    err = configureUart();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART initialization failed: %s", esp_err_to_name(err));
        return;
    }

    TaskHandle_t led_task_handle = NULL;
    if (xTaskCreate(task_Led_500ms, "task_led_blink", 1024, NULL, 5,
                    &led_task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED task");
        return;
    }

    err = initWifi();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi initialization failed: %s", esp_err_to_name(err));
        vTaskDelete(led_task_handle);
        return;
    }

    if (wifiIsConnected()) {
        if (!tcp_server_start()) {
            ESP_LOGE(TAG, "Failed to start TCP server");
        }
    }
}

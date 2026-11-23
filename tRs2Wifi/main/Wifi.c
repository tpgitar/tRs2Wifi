#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "Led.h"
#include "Wifi.h"
#include "tcpServer.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY 10
#define WIFI_INITIAL_WAIT_MS 30000
#define WIFI_RETRY_DELAY_MS 5000
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num;
static bool s_connected_once;
static bool s_wifi_connected;
static esp_timer_handle_t s_retry_timer;

static void wifi_schedule_retry(void);

static void wifi_retry_timer_callback(void *arg)
{
    (void)arg;
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Delayed Wi-Fi reconnect failed: %s", esp_err_to_name(err));
        wifi_schedule_retry();
    } else {
        ESP_LOGI(TAG, "Retrying Wi-Fi connection");
    }
}

static void wifi_schedule_retry(void)
{
    if (s_retry_timer == NULL) {
        const esp_timer_create_args_t timer_args = {
            .callback = wifi_retry_timer_callback,
            .name = "wifi_retry",
        };
        esp_err_t err = esp_timer_create(&timer_args, &s_retry_timer);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create Wi-Fi retry timer: %s", esp_err_to_name(err));
            return;
        }
    }

    esp_err_t err = esp_timer_stop(s_retry_timer);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Failed to stop Wi-Fi retry timer: %s", esp_err_to_name(err));
    }
    err = esp_timer_start_once(s_retry_timer, WIFI_RETRY_DELAY_MS * 1000ULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to schedule Wi-Fi retry: %s", esp_err_to_name(err));
    }
}

static void wifi_cancel_retry(void)
{
    if (s_retry_timer != NULL) {
        esp_err_t err = esp_timer_stop(s_retry_timer);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "Failed to cancel Wi-Fi retry timer: %s", esp_err_to_name(err));
        }
    }
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_err_t err = esp_wifi_connect();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Initial Wi-Fi connection failed: %s", esp_err_to_name(err));
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        setLedBlue();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_connected = false;
        setLedRed();
        tcp_server_request_stop();
        if (s_connected_once) {
            ESP_LOGI(TAG, "Wi-Fi disconnected; scheduling reconnect");
            wifi_schedule_retry();
            return;
        }

        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            s_retry_num++;
            esp_err_t err = esp_wifi_connect();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Wi-Fi reconnect failed: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "Retrying Wi-Fi connection (%d/%d)",
                         s_retry_num, EXAMPLE_ESP_MAXIMUM_RETRY);
            }
        } else {
            ESP_LOGW(TAG, "Wi-Fi connection attempts exhausted; waiting for retry");
            wifi_schedule_retry();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_wifi_connected = true;
        wifi_cancel_retry();
        setLedGreen();
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_connected_once = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        if (!tcp_server_start()) {
            ESP_LOGE(TAG, "Could not start TCP server after Wi-Fi reconnect");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
        s_wifi_connected = false;
        setLedRed();
        tcp_server_request_stop();
        ESP_LOGW(TAG, "Wi-Fi IP address lost");
    }
}

static esp_err_t wifi_init_sta(void)
{
    if (CONFIG_ESP_WIFI_SSID[0] == '\0') {
        ESP_LOGE(TAG, "Wi-Fi SSID is not configured");
        return ESP_ERR_INVALID_ARG;
    }

    EventGroupHandle_t event_group = xEventGroupCreate();
    if (event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
        return ESP_ERR_NO_MEM;
    }
    s_wifi_event_group = event_group;

    esp_event_handler_instance_t instance_any_id = NULL;
    esp_event_handler_instance_t instance_got_ip = NULL;
    esp_event_handler_instance_t instance_lost_ip = NULL;
    esp_netif_t *sta_netif = NULL;
    bool wifi_initialized = false;
    bool wifi_started = false;
    esp_err_t err;

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Network interface initialization failed: %s", esp_err_to_name(err));
        goto fail;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Default event loop creation failed: %s", esp_err_to_name(err));
        goto fail;
    }

    sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi station interface");
        err = ESP_FAIL;
        goto fail;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi driver initialization failed: %s", esp_err_to_name(err));
        goto fail;
    }
    wifi_initialized = true;

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                              &event_handler, NULL, &instance_any_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi event handler registration failed: %s", esp_err_to_name(err));
        goto fail;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                              &event_handler, NULL, &instance_got_ip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "IP event handler registration failed: %s", esp_err_to_name(err));
        goto fail;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_LOST_IP,
                                              &event_handler, NULL, &instance_lost_ip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Lost-IP event handler registration failed: %s", esp_err_to_name(err));
        goto fail;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = CONFIG_ESP_WIFI_PASSWORD[0] == '\0'
                                      ? WIFI_AUTH_OPEN
                                      : WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Setting Wi-Fi station mode failed: %s", esp_err_to_name(err));
        goto fail;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Setting Wi-Fi configuration failed: %s", esp_err_to_name(err));
        goto fail;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Starting Wi-Fi failed: %s", esp_err_to_name(err));
        goto fail;
    }
    wifi_started = true;

    EventBits_t bits = xEventGroupWaitBits(event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE,
                                           pdMS_TO_TICKS(WIFI_INITIAL_WAIT_MS));
    if ((bits & WIFI_CONNECTED_BIT) != 0) {
        return ESP_OK;
    }
    ESP_LOGW(TAG, "Wi-Fi did not obtain IP within %d ms; continuing reconnect attempts",
             WIFI_INITIAL_WAIT_MS);
    return ESP_OK;

fail:
    if (wifi_started) {
        esp_err_t cleanup_err = esp_wifi_stop();
        if (cleanup_err != ESP_OK) {
            ESP_LOGE(TAG, "Stopping Wi-Fi after initialization failure failed: %s",
                     esp_err_to_name(cleanup_err));
        }
    }
    if (instance_got_ip != NULL) {
        esp_err_t cleanup_err = esp_event_handler_instance_unregister(
            IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
        if (cleanup_err != ESP_OK) {
            ESP_LOGE(TAG, "Unregistering IP event handler failed: %s",
                     esp_err_to_name(cleanup_err));
        }
    }
    if (instance_lost_ip != NULL) {
        esp_err_t cleanup_err = esp_event_handler_instance_unregister(
            IP_EVENT, IP_EVENT_STA_LOST_IP, instance_lost_ip);
        if (cleanup_err != ESP_OK) {
            ESP_LOGE(TAG, "Unregistering lost-IP event handler failed: %s",
                     esp_err_to_name(cleanup_err));
        }
    }
    if (instance_any_id != NULL) {
        esp_err_t cleanup_err = esp_event_handler_instance_unregister(
            WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
        if (cleanup_err != ESP_OK) {
            ESP_LOGE(TAG, "Unregistering Wi-Fi event handler failed: %s",
                     esp_err_to_name(cleanup_err));
        }
    }
    if (wifi_initialized) {
        esp_err_t cleanup_err = esp_wifi_deinit();
        if (cleanup_err != ESP_OK) {
            ESP_LOGE(TAG, "Deinitializing Wi-Fi after initialization failure failed: %s",
                     esp_err_to_name(cleanup_err));
        }
    }
    if (sta_netif != NULL) {
        esp_netif_destroy_default_wifi(sta_netif);
    }
    s_wifi_event_group = NULL;
    vEventGroupDelete(event_group);
    return err;
}

esp_err_t initWifi(void)
{
    setLedRed();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erasing NVS failed: %s", esp_err_to_name(err));
            return err;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(err));
        return err;
    }

    return wifi_init_sta();
}

bool wifiIsConnected(void)
{
    return s_wifi_connected;
}

/*
 * Led.c
 *
 *  Created on: 26 lis 2025
 *      Author: t_pro
 */
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "Led.h"
//-------------------------------------------------------------------------------------

static const char *TAG = "LED";
static led_strip_handle_t led_strip;
static portMUX_TYPE led_state_lock = portMUX_INITIALIZER_UNLOCKED;
static bool tcp_connection_active;

#define BLINK_GPIO 8
#define CONFIG_BLINK_LED_STRIP_BACKEND_RMT 1
#define LED_BLINK_INTERVAL_MS 300
#define LED_TCP_BLINK_INTERVAL_MS (LED_BLINK_INTERVAL_MS / 2)




//-------------------------------------------------------------------------------------
struct def_led_color
{
	uint16_t unRed;
	uint16_t unGreen;
	uint16_t unBlue;
};

static struct def_led_color Led = {0, 0, 0};

//-------------------------------------------------------------------------------------
void setLedGreen()
{
	portENTER_CRITICAL(&led_state_lock);
	Led.unBlue = 0;
	Led.unGreen = 16;
	Led.unRed = 0;
	portEXIT_CRITICAL(&led_state_lock);
} 

void setLedBlue()
{
	portENTER_CRITICAL(&led_state_lock);
	Led.unBlue = 16;
	Led.unGreen = 0;
	Led.unRed = 0;
	portEXIT_CRITICAL(&led_state_lock);
} 


void setLedRed()
{
	portENTER_CRITICAL(&led_state_lock);
	Led.unBlue = 0;
	Led.unGreen = 0;
	Led.unRed = 16;
	portEXIT_CRITICAL(&led_state_lock);
} 

void setLedTcpConnectionActive(bool active)
{
	portENTER_CRITICAL(&led_state_lock);
	tcp_connection_active = active;
	portEXIT_CRITICAL(&led_state_lock);
}

static bool is_tcp_connection_active(void)
{
	portENTER_CRITICAL(&led_state_lock);
	bool active = tcp_connection_active;
	portEXIT_CRITICAL(&led_state_lock);
	return active;
}




//-------------------------------------------------------------------------------------



void blink_led(void)
{
	static uint8_t s_led_state = 0;
	struct def_led_color led;
	portENTER_CRITICAL(&led_state_lock);
	led = Led;
	portEXIT_CRITICAL(&led_state_lock);

    if (s_led_state) {
        led_strip_set_pixel(led_strip, 0, led.unRed, led.unGreen, led.unBlue);
        led_strip_refresh(led_strip);
    } else {
        led_strip_clear(led_strip);
    }
	s_led_state = s_led_state ? 0 : 1;
}
//-------------------------------------------------------------------------------------
esp_err_t configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    esp_err_t err = led_strip_new_spi_device(&strip_config, &spi_config, &led_strip);
#else
#error "unsupported LED strip backend"
#endif
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED strip initialization failed: %s", esp_err_to_name(err));
        return err;
    }

    err = led_strip_clear(led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear LED strip: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
//-------------------------------------------------------------------------------------
void task_Led_500ms(void *pvParameters)
{
	(void)pvParameters;
	const TickType_t tick_period = pdMS_TO_TICKS(50);
	TickType_t last_wake = xTaskGetTickCount();
	TickType_t elapsed_ticks = 0;
    while (1)
    {
		uint32_t interval_ms = is_tcp_connection_active()
		                           ? LED_TCP_BLINK_INTERVAL_MS
		                           : LED_BLINK_INTERVAL_MS;
		TickType_t interval_ticks = pdMS_TO_TICKS(interval_ms);
		if (elapsed_ticks >= interval_ticks) {
			blink_led();
			elapsed_ticks = 0;
		}
		vTaskDelayUntil(&last_wake, tick_period);
		elapsed_ticks += tick_period;
    }
}


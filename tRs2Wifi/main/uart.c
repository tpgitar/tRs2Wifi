/*
 * uart.c
 *
 *  Created on: 4 gru 2025
 *      Author: t_pro
 */

#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "uart.h"

//----------------------------------------------------------------

#define CONFIG_EXAMPLE_UART_RXD 5
#define CONFIG_EXAMPLE_UART_TXD 4

#define UART_PIN_NO_CHANGE      (-1)

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define CONFIG_EXAMPLE_UART_BAUD_RATE 57600
#define CONFIG_EXAMPLE_UART_PORT_NUM 1

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
//#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART";

#define BUF_SIZE 1024

esp_err_t configureUart(void)
{
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    esp_err_t err = uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0,
                                        NULL, intr_alloc_flags);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART driver installation failed: %s", esp_err_to_name(err));
        return err;
    }

    err = uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART parameter configuration failed: %s", esp_err_to_name(err));
        uart_driver_delete(ECHO_UART_PORT_NUM);
        return err;
    }

    err = uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD,
                       ECHO_TEST_RTS, ECHO_TEST_CTS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART pin configuration failed: %s", esp_err_to_name(err));
        uart_driver_delete(ECHO_UART_PORT_NUM);
        return err;
    }

    return ESP_OK;
}
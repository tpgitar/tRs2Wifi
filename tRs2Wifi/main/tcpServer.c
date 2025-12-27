/*
 * tcpServer.c
 *
 *  Created on: 2 gru 2025
 *      Author: t_pro
 */

#include <stdint.h>
#include <string.h>
/*#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
*/
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "driver/uart.h"

//-------------------------------------------------------------------------------------------------
static TaskHandle_t uart_to_tcp_handle = NULL;
static TaskHandle_t tcp_to_uart_handle = NULL;

uint8_t ucTcpToUartStatus = 0;
uint8_t ucUartToTcpStatus = 0;


static const char *TAG = "TCP server";


#define TCP_PORT 2000


#define UART_BUF_SIZE 1024
#define UART_PORT 1


uint16_t unBreak = 0;

//-------------------------------------------------------------------------------------------------
static void tcp_to_uart_task(void *arg)
{
	
	
	int sock = (int)(intptr_t)arg;
	uint8_t *data = malloc(UART_BUF_SIZE);
	if (!data) 
	{
		ESP_LOGE(TAG, "malloc failed");
		vTaskDelete(NULL);
		return;
	}

	while (ucUartToTcpStatus) 
	{
		int len = recv(sock, data, UART_BUF_SIZE, 0);

		//ESP_LOGW(TAG, "Data read");           //>>>>>>>>>>>>>>>>>>>>>>
		
	
		if (len > 0) 
		{
			
			int written = uart_write_bytes(UART_PORT, (const char *)data, len);
			if (written < 0) 
			{
				ESP_LOGW(TAG, "uart_write_bytes failed");
				break;
			}
		} 
		else
		{
		 	if (len == 0) 
		    {
				ESP_LOGI(TAG, "Client closed connection (recv returned 0)");
				break;
			}
			else
			{
				ESP_LOGW(TAG, "recv() failed");
				break;
			}
		}
		
		
		
	}

	free(data);
	ucTcpToUartStatus = 0;
	
	/*
	if (uart_to_tcp_handle) vTaskDelete(uart_to_tcp_handle);
	uart_to_tcp_handle = NULL;
	*/
	
	tcp_to_uart_handle = NULL;
	ESP_LOGI(TAG, "tcp_to_uart_task exiting");
	vTaskDelete(NULL);
}
//-------------------------------------------------------------------------------------------------
static void uart_to_tcp_task(void *arg)
{
	int sock = (int)(intptr_t)arg;
	uint8_t *data = malloc(UART_BUF_SIZE);
	
	if (!data) 
	{
		ESP_LOGE(TAG, "malloc failed");
		vTaskDelete(NULL);
		return;
	}


	while (ucTcpToUartStatus) 
	{
	
		
		int len = uart_read_bytes(UART_PORT, data, UART_BUF_SIZE, pdMS_TO_TICKS(100));
		if (len > 0)
		{
		//	ESP_LOGW(TAG, "Data read from uart - sending via TCP");                       //>>>>>>>>>>>>
			int sent = send(sock, data, len, 0);
			if (sent < 0)
		 	{
				ESP_LOGW(TAG, "send() failed — client probably disconnected");
				break;
			}
		}		
		
	
	/*
		
		char sTest[] = "Send\r\n";
		
		ESP_LOGW(TAG, "Send 1");
		if(send(sock, sTest, strlen(sTest), 0) < 0)
		{
			ESP_LOGW(TAG, "brak;");
			unBreak = 1;
			
			break;
		}
		
		ESP_LOGW(TAG, "Send 2");
		
		// Optional: yield to allow other task to run
		vTaskDelay(pdMS_TO_TICKS(1000));
		
		*/
		
	}


	free(data);
	// notify other task that connection ended by cancelling it
	/*
	if (tcp_to_uart_handle) vTaskDelete(tcp_to_uart_handle);
	tcp_to_uart_handle = NULL;
	*/
	ucUartToTcpStatus = 0;
	
	uart_to_tcp_handle = NULL;
	ESP_LOGI(TAG, "uart_to_tcp_task exiting");
	vTaskDelete(NULL);
}
//-------------------------------------------------------------------------------------------------


void tcp_server_task(void *pvParameters)
{
	struct sockaddr_in server_addr;
	int listen_sock = -1;


	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listen_sock < 0) 
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}


	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(TCP_PORT);


	int err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err != 0)
	{
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		close(listen_sock);
		vTaskDelete(NULL);
		return;
	}


	err = listen(listen_sock, 1);
	if (err != 0) 
	{
		ESP_LOGE(TAG, "Error during listen: errno %d", errno);
		close(listen_sock);
		vTaskDelete(NULL);
		return;
	}


	while (1)
	{
		ESP_LOGI(TAG, "Waiting for a TCP client on port %d...", TCP_PORT);
		
		struct sockaddr_in6 client_addr;
		uint addr_len = sizeof(client_addr);
		int sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
		if (sock < 0) 
		{
			ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			continue;
		}


		ESP_LOGI(TAG, "Client connected, socket=%d", sock);


		// increase RX timeout to detect stalled clients (optional)
		struct timeval tv;
		tv.tv_sec = 60; // seconds
		tv.tv_usec = 0;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


		// create two tasks for bidirectional forwarding
		unBreak = 0;
		
		ucTcpToUartStatus = 1;
		ucUartToTcpStatus = 1;
			
		xTaskCreate(uart_to_tcp_task, "uart_to_tcp", 4096, (void *)(intptr_t)sock, 5, &uart_to_tcp_handle);
		xTaskCreate(tcp_to_uart_task, "tcp_to_uart", 4096, (void *)(intptr_t)sock, 5, &tcp_to_uart_handle);


		// Wait until both tasks exit (they delete themselves)
		while (uart_to_tcp_handle != NULL || tcp_to_uart_handle != NULL) 
		{
			vTaskDelay(pdMS_TO_TICKS(100));
		}


		// Shutdown and close socket
		shutdown(sock, SHUT_RDWR);
		close(sock);
		ESP_LOGI(TAG, "Client socket closed, returning to accept()");
	}


	// never reached in normal operation
	close(listen_sock);
	vTaskDelete(NULL);
}

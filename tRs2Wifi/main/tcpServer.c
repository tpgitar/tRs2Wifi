/*
 * tcpServer.c
 *
 *  Created on: 2 gru 2025
 *      Author: t_pro
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "driver/uart.h"
#include "tcpServer.h"
#include "Led.h"

static const char *TAG = "TCP server";
#define TCP_PORT 2000
#define UART_BUF_SIZE 1024
#define UART_PORT 1
#define TCP_SESSION_IDLE_TIMEOUT_MS 60000
#define TCP_SESSION_RECV_TIMEOUT_MS 1000
#define TCP_TO_UART_DONE_BIT (1U << 0)
#define UART_TO_TCP_DONE_BIT (1U << 1)
#define SESSION_STOP_BIT (1U << 2)

typedef struct {
	int socket;
	EventGroupHandle_t events;
	portMUX_TYPE activity_lock;
	TickType_t last_activity_tick;
} tcp_session_t;

static tcp_session_t *s_active_session;
static portMUX_TYPE s_active_session_lock = portMUX_INITIALIZER_UNLOCKED;
static TaskHandle_t s_tcp_server_task_handle;

static void tcp_session_mark_activity(tcp_session_t *session)
{
	portENTER_CRITICAL(&session->activity_lock);
	session->last_activity_tick = xTaskGetTickCount();
	portEXIT_CRITICAL(&session->activity_lock);
}

static bool tcp_session_idle_timeout_expired(tcp_session_t *session)
{
	portENTER_CRITICAL(&session->activity_lock);
	TickType_t idle_ticks = xTaskGetTickCount() - session->last_activity_tick;
	portEXIT_CRITICAL(&session->activity_lock);
	return idle_ticks >= pdMS_TO_TICKS(TCP_SESSION_IDLE_TIMEOUT_MS);
}

static void tcp_close_socket(int sock, const char *description)
{
	if (close(sock) != 0) {
		ESP_LOGW(TAG, "close(%s) failed: errno %d", description, errno);
	}
}

static bool tcp_session_should_stop(const tcp_session_t *session)
{
	return (xEventGroupGetBits(session->events) & SESSION_STOP_BIT) != 0;
}

static void tcp_session_request_stop(tcp_session_t *session)
{
	xEventGroupSetBits(session->events, SESSION_STOP_BIT);
	if (shutdown(session->socket, SHUT_RDWR) != 0 && errno != ENOTCONN) {
		ESP_LOGW(TAG, "shutdown() failed: errno %d", errno);
	}
}

void tcp_server_request_stop(void)
{
	portENTER_CRITICAL(&s_active_session_lock);
	tcp_session_t *session = s_active_session;
	portEXIT_CRITICAL(&s_active_session_lock);

	if (session != NULL) {
		ESP_LOGI(TAG, "Stopping active TCP session");
		tcp_session_request_stop(session);
	}
}

bool tcp_server_start(void)
{
	if (s_tcp_server_task_handle != NULL) {
		return true;
	}
	if (xTaskCreate(tcp_server_task, "tcp_server", 8192, NULL, 5,
	                &s_tcp_server_task_handle) != pdPASS) {
		s_tcp_server_task_handle = NULL;
		ESP_LOGE(TAG, "Failed to create TCP server task");
		return false;
	}
	return true;
}

static void tcp_server_set_active_session(tcp_session_t *session)
{
	portENTER_CRITICAL(&s_active_session_lock);
	s_active_session = session;
	portEXIT_CRITICAL(&s_active_session_lock);
}

static void tcp_session_finish(tcp_session_t *session, EventBits_t done_bit)
{
	tcp_session_request_stop(session);
	xEventGroupSetBits(session->events, done_bit);
	vTaskDelete(NULL);
}

static bool uart_write_all(tcp_session_t *session, const uint8_t *data, int len)
{
	int offset = 0;
	int no_progress_delay_count = 0;
	while (offset < len && !tcp_session_should_stop(session)) {
		int written = uart_write_bytes(UART_PORT, (const char *)data + offset, len - offset);
		if (written > 0) {
			offset += written;
			no_progress_delay_count = 0;
			continue;
		}
		if (written < 0) {
			ESP_LOGE(TAG, "UART write failed after %d of %d bytes", offset, len);
			return false;
		}

		vTaskDelay(pdMS_TO_TICKS(10));
		no_progress_delay_count++;
		if (no_progress_delay_count == 1 || no_progress_delay_count % 50 == 0) {
			ESP_LOGW(TAG, "UART TX write made no progress for %d bytes of %d",
			         len - offset, len);
		}
		if (no_progress_delay_count >= 500) {
			ESP_LOGE(TAG, "UART TX buffer did not drain after 5 s");
			return false;
		}
	}
	return offset == len;
}

static void tcp_to_uart_task(void *arg)
{
	tcp_session_t *session = arg;
	uint8_t *data = malloc(UART_BUF_SIZE);
	if (data == NULL) {
		ESP_LOGE(TAG, "Failed to allocate TCP-to-UART buffer");
		tcp_session_finish(session, TCP_TO_UART_DONE_BIT);
		return;
	}
	ESP_LOGI(TAG, "TCP-to-UART task started");

	while (!tcp_session_should_stop(session)) {
		int len = recv(session->socket, data, UART_BUF_SIZE, 0);
		if (len > 0) {
			tcp_session_mark_activity(session);
			if (!uart_write_all(session, data, len)) {
				ESP_LOGE(TAG, "UART write did not complete");
				break;
			}
		} else if (len == 0) {
			ESP_LOGI(TAG, "Client closed the TCP connection");
			break;
		} else if (errno == EINTR) {
			continue;
		} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
			if (tcp_session_idle_timeout_expired(session)) {
				ESP_LOGI(TAG, "TCP session idle timeout");
				break;
			}
			continue;
		} else {
			ESP_LOGW(TAG, "recv() failed: errno %d", errno);
			break;
		}
	}

	free(data);
	ESP_LOGI(TAG, "TCP-to-UART task exiting");
	tcp_session_finish(session, TCP_TO_UART_DONE_BIT);
}

static void uart_to_tcp_task(void *arg)
{
	tcp_session_t *session = arg;
	uint8_t *data = malloc(UART_BUF_SIZE);
	if (data == NULL) {
		ESP_LOGE(TAG, "Failed to allocate UART-to-TCP buffer");
		tcp_session_finish(session, UART_TO_TCP_DONE_BIT);
		return;
	}
	ESP_LOGI(TAG, "UART-to-TCP task started");

	while (!tcp_session_should_stop(session)) {
		size_t available = 0;
		esp_err_t err = uart_get_buffered_data_len(UART_PORT, &available);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "uart_get_buffered_data_len failed: %s", esp_err_to_name(err));
			break;
		}
		if (available == 0) {
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}
		if (available >= (UART_BUF_SIZE * 2 - UART_BUF_SIZE / 2)) {
			ESP_LOGW(TAG, "UART RX buffer pressure: %u bytes buffered", (unsigned)available);
		}

		uint32_t read_size = available > UART_BUF_SIZE ? UART_BUF_SIZE : (uint32_t)available;
		int len = uart_read_bytes(UART_PORT, data, read_size, 0);
		if (len < 0) {
			ESP_LOGE(TAG, "uart_read_bytes failed");
			break;
		}
		if (len == 0) {
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}

		int offset = 0;
		while (offset < len && !tcp_session_should_stop(session)) {
			int sent = send(session->socket, data + offset, len - offset, 0);
			if (sent > 0) {
				offset += sent;
				tcp_session_mark_activity(session);
			} else if (sent < 0 && errno == EINTR) {
				continue;
			} else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK) &&
			           !tcp_session_idle_timeout_expired(session)) {
				continue;
			} else {
				if (sent == 0) {
					ESP_LOGW(TAG, "send() made no progress after %d of %d bytes",
					         offset, len);
				} else {
					ESP_LOGW(TAG, "send() failed after %d of %d bytes: errno %d",
					         offset, len, errno);
				}
				free(data);
				tcp_session_finish(session, UART_TO_TCP_DONE_BIT);
				return;
			}
		}
	}

	free(data);
	ESP_LOGI(TAG, "UART-to-TCP task exiting");
	tcp_session_finish(session, UART_TO_TCP_DONE_BIT);
}

void tcp_server_task(void *pvParameters)
{
	(void)pvParameters;

	struct sockaddr_in server_addr = {0};
	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listen_sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(TCP_PORT);

	if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		tcp_close_socket(listen_sock, "listen socket");
		vTaskDelete(NULL);
		return;
	}

	if (listen(listen_sock, 1) != 0) {
		ESP_LOGE(TAG, "Error during listen: errno %d", errno);
		tcp_close_socket(listen_sock, "listen socket");
		vTaskDelete(NULL);
		return;
	}

	while (true) {
		ESP_LOGI(TAG, "Waiting for a TCP client on port %d...", TCP_PORT);

		struct sockaddr_storage client_addr;
		socklen_t addr_len = sizeof(client_addr);
		int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
		if (client_sock < 0) {
			if (errno != EINTR) {
				ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			}
			continue;
		}

		ESP_LOGI(TAG, "Client connected, socket=%d", client_sock);

		struct timeval timeout = {
			.tv_sec = TCP_SESSION_RECV_TIMEOUT_MS / 1000,
			.tv_usec = (TCP_SESSION_RECV_TIMEOUT_MS % 1000) * 1000,
		};
		if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO,
		               &timeout, sizeof(timeout)) != 0) {
			ESP_LOGE(TAG, "setsockopt(SO_RCVTIMEO) failed: errno %d", errno);
			tcp_close_socket(client_sock, "client socket");
			continue;
		}
		timeout.tv_sec = 5;
		if (setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO,
		               &timeout, sizeof(timeout)) != 0) {
			ESP_LOGE(TAG, "setsockopt(SO_SNDTIMEO) failed: errno %d", errno);
			tcp_close_socket(client_sock, "client socket");
			continue;
		}

		tcp_session_t session = {
			.socket = client_sock,
			.events = xEventGroupCreate(),
		};
		portMUX_INITIALIZE(&session.activity_lock);
		session.last_activity_tick = xTaskGetTickCount();
		if (session.events == NULL) {
			ESP_LOGE(TAG, "Failed to create TCP session event group");
			tcp_close_socket(client_sock, "client socket");
			continue;
		}
		tcp_server_set_active_session(&session);

		if (xTaskCreate(uart_to_tcp_task, "uart_to_tcp", 4096,
		                &session, 5, NULL) != pdPASS) {
			ESP_LOGE(TAG, "Failed to create UART-to-TCP task");
			tcp_close_socket(client_sock, "client socket");
			tcp_server_set_active_session(NULL);
			vEventGroupDelete(session.events);
			continue;
		}

		if (xTaskCreate(tcp_to_uart_task, "tcp_to_uart", 4096,
		                &session, 5, NULL) != pdPASS) {
			ESP_LOGE(TAG, "Failed to create TCP-to-UART task");
			tcp_session_request_stop(&session);
			xEventGroupWaitBits(session.events, UART_TO_TCP_DONE_BIT,
			                    pdFALSE, pdTRUE, portMAX_DELAY);
		} else {
			setLedTcpConnectionActive(true);
			xEventGroupWaitBits(session.events,
			                    TCP_TO_UART_DONE_BIT | UART_TO_TCP_DONE_BIT,
			                    pdFALSE, pdTRUE, portMAX_DELAY);
			setLedTcpConnectionActive(false);
		}

		tcp_close_socket(client_sock, "client socket");
		tcp_server_set_active_session(NULL);
		setLedTcpConnectionActive(false);
		vEventGroupDelete(session.events);
		ESP_LOGI(TAG, "Client session closed, returning to accept()");
	}
}

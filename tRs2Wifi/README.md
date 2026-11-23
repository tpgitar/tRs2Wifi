# tRs2Wifi

RS232-to-Wi-Fi adapter based on ESP32-C3-MINI-1 and ESP-IDF 6.1.

## Communication

- TCP server listens on all IPv4 interfaces on port `2000`.
- UART1 uses `57600 8N1`, TX GPIO4 and RX GPIO5.
- A TCP client is bridged to UART1 in both directions.
- One TCP session is supported at a time.
- TCP is a byte stream and has no application-level message boundaries.
- An idle session closes after 60 seconds without traffic in either direction.

## Wi-Fi

Configure `ESP_WIFI_SSID` and `ESP_WIFI_PASSWORD` in ESP-IDF:

```text
idf.py menuconfig
```

The values are stored in ignored local `sdkconfig.local`. Do not publish this file or credentials.

Startup retries Wi-Fi 10 times and then retries every 5 seconds. Losing Wi-Fi or its IP address stops an active TCP session and reconnects automatically.

## LED status

- Red: no active Wi-Fi connection.
- Blue: associated with the access point before IP acquisition.
- Green: connected and IP address acquired.
- When TCP is connected, the LED blink period is 150 ms instead of 300 ms.

## Build

```powershell
cd tRs2Wifi
idf.py set-target esp32c3
idf.py build
idf.py -p COM7 flash monitor
```

## Tests

Run the TCP acceptance tests from the project directory:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\Copilot\Test\run_tests.ps1" -Address <device-ip> -Port 2000
```

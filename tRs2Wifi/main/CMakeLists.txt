# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c Led.c Wifi.c tcpServer.c uart.c
    PRIV_REQUIRES
        esp_common
        esp_event
        esp_driver_gpio
        esp_driver_uart
        esp_hw_support
        esp_netif
        esp_system
        esp_timer
        esp_wifi
        freertos
        led_strip
        log
        lwip
        nvs_flash
)

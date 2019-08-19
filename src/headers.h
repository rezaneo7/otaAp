#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_image_format.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "cJSON.h"
#include "driver/gpio.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/priv/api_msg.h"
#include "lwip/priv/tcp_priv.h"
#include "mdns.h"

#include "esp_log.h"


#include "wifi_functions.h"




static const char *MAIN_TAG = "ESP32";
static const char* WIFI_TAG = "ESP_WIFI";
static const char* SERVER_TAG = "ESP_SERVER";
#define SSID "ESP32AP"
#define OTA_BUF_SIZE 256


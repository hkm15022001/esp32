#ifndef _HEADER_H_
#define _HEADER_H_
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"

#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "esp_log.h"
//#include "esp_mac.h"
#include "esp_sntp.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_https_ota.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#define FIRMWARE_VER "1.0.0"
#define HARDWARE_VER "1.0.0"
#define IDSTR "%04x"
#define MQTT_BROKER "mqtt.innoway.vn"
#define TOPIC_STATUS "messages/f4a7ce30-0379-4a28-97ad-a607f02b96b4/status"
#define TOPIC_UPDATE "messages/f4a7ce30-0379-4a28-97ad-a607f02b96b4/update"
#define HEARTBEAT_TIME 60

#define BUTTON_TRIGGER 0
#define BUTTON_NOT_TRIGGER 1

#define TIME_CLICK_MIN (20 / portTICK_RATE_MS)
#define TIME_CLICK_MAX (1000 / portTICK_RATE_MS)
#define TIME_CHECK (1000 / portTICK_RATE_MS)

typedef enum 
{
    DISCONNECTED,
    WIFI_CONNECTED,
} wireless_state_t;

typedef struct
{
    uint32_t time_down;
    uint8_t pin;
    uint32_t time_up;
    uint32_t deltaT;
    uint8_t click_cnt;
    uint32_t time_stamp;
} button_t;

#endif
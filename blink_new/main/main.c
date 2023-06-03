#include "header.h"

#include "common_user.h"
#include "mqtt_user.h"
#include "wifi_user.h"
#include "smartcfg.h"

static const char *TAG = "MAIN";
RTC_NOINIT_ATTR gateway_mode_t mode;
wireless_state_t wireless_state = DISCONNECTED;
extern esp_mqtt_client_handle_t client;
TimerHandle_t hbTimerHandle;
TimerHandle_t ledChangeTimerHandle;

void hbTimerCb(TimerHandle_t hbTimerHandle)
{
    if (wireless_state == WIFI_CONNECTED)
    {
        esp_mqtt_client_publish(client, TOPIC_UPDATE, "{\"heartbeat\":1}", strlen("{\"heartbeat\":1}"), 0, 0);
    }
}

void ledChangeTimerCb(TimerHandle_t ledChangeTimerHandle)
{
    if (wireless_state == WIFI_CONNECTED)
    {
        uint8_t ledState = (GPIO_REG_READ(GPIO_OUT_REG) >> GPIO_NUM_2) & 1U;
        uint8_t status[64] = {0};
        sprintf((char *)status, "{\"led\":2,\"status\":\"%s\"}", (ledState == 1) ? "on" : "off");
        esp_mqtt_client_publish(client, TOPIC_UPDATE, (const char *)status, strlen((char *)status), 0, 0);
    }
}

void app_main(void)
{
    esp_err_t err;
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    wifi_init();
    if (mode == SMARTCONFIG)
    {
        smartconfig_init();
    }
    else
    {
        hbTimerHandle = xTimerCreate("Heartbeat", 60000 / portTICK_RATE_MS, pdTRUE, (void *)0, hbTimerCb);
        xTimerStart(hbTimerHandle, 0);
        wifi_config_t wifi_cfg = {
            .sta = {
                .pmf_cfg = {
                    .capable = true,
                    .required = false,
                },
            },
        };
        if (esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg) == ESP_OK)
        {
            if (strlen((char *)wifi_cfg.sta.ssid) > 0)
            {
                ESP_LOGI(TAG, "Wifi configuration already stored in flash partition called NVS");
                ESP_LOGI(TAG, "%s", wifi_cfg.sta.ssid);
                ESP_LOGI(TAG, "%s", wifi_cfg.sta.password);
                wifi_sta_init(wifi_cfg, WIFI_MODE_STA);
                mqtt_client_init();
            }
            else
            {
                ESP_LOGE(TAG, "Cannot read WiFi info");
            }
        }
    }

    button_t button = {
        .pin = GPIO_NUM_0,
        .time_down = 0,
        .time_up = 0,
        .deltaT = 0,
        .click_cnt = 0,
        .time_stamp = 0,
    };

    gpio_config_t led_cfg = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .pin_bit_mask = (1ULL << GPIO_NUM_2)};
    gpio_config(&led_cfg);
    gpio_set_level(GPIO_NUM_2, 0);

    while (1)
    {
        if (gpio_get_level(button.pin) == BUTTON_TRIGGER)
        {
            if (button.time_up == 0)
                button.time_up = (uint32_t)(xTaskGetTickCount() / portTICK_RATE_MS);
            else
            {
                button.deltaT = (uint32_t)(xTaskGetTickCount() / portTICK_RATE_MS) - button.time_up;
            }
        }
        else if (gpio_get_level(button.pin) == BUTTON_NOT_TRIGGER && button.time_up != 0 && button.deltaT > TIME_CLICK_MIN)
        {
            button.time_down = (uint32_t)(xTaskGetTickCount() / portTICK_RATE_MS);
            button.deltaT = button.time_down - button.time_up;
            // ESP_LOGI(TAG, "DeltaT: %d", button.deltaT);
            if (button.deltaT > TIME_CLICK_MIN && button.deltaT < TIME_CLICK_MAX)
            {
                button.click_cnt++;
                ESP_LOGI(TAG, "Button counter: %d", button.click_cnt);
                button.time_stamp = button.time_up;
                button.time_up = 0;
                button.time_down = 0;
                button.deltaT = 0;
            }
        }
        else if (gpio_get_level(button.pin) == BUTTON_NOT_TRIGGER && (uint32_t)(xTaskGetTickCount() / portTICK_RATE_MS) - button.time_stamp > TIME_CHECK && button.time_stamp != 0)
        {
            if (button.click_cnt == 2)
            {
                uint8_t ledState = (GPIO_REG_READ(GPIO_OUT_REG) >> GPIO_NUM_2) & 1U;
                ledState = 1 - ledState;
                gpio_set_level(GPIO_NUM_2, ledState);
                ledChangeTimerHandle = xTimerCreate("Led", 20000 / portTICK_RATE_MS, pdTRUE, (void *)0, ledChangeTimerCb);
                xTimerStart(ledChangeTimerHandle, 0);
            }
            else if (button.click_cnt == 4)
            {
                mode = SMARTCONFIG;
                esp_restart();
            }
            button.time_up = 0;
            button.time_down = 0;
            button.deltaT = 0;
            button.click_cnt = 0;
            button.time_stamp = 0;
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

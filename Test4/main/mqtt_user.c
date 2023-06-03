#include "mqtt_user.h"
#include "common_user.h"

static const char *TAG = "MQTT";
RingbufHandle_t mqtt_ring_buf;
esp_mqtt_client_handle_t client;
extern wireless_state_t wireless_state;
extern TimerHandle_t ledChangeTimerHandle;

void mqtt_client_init(void)
{
    uint8_t broker[50] = {0};
    sprintf((char *)broker, "mqtt://%s", MQTT_BROKER);
    mqtt_ring_buf = xRingbufferCreate(4096, RINGBUF_TYPE_NOSPLIT);
    if (mqtt_ring_buf == NULL)
        ESP_LOGE(TAG, "Failed to create ring buffer");
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = (char *)broker,
        .password = "Q1uIDvGAJD4YO48B29KICC0xKodyQsHa",
        .keepalive = 60,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    xTaskCreate(&mqtt_user_task, "MQTT", 4096, NULL, 9, NULL);
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        ESP_LOGI(TAG, "MQTT event connected");
        esp_mqtt_client_subscribe(client, TOPIC_STATUS, 0);
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT event subcribed, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT event unsubcribed, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT event published, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        UBaseType_t res = xRingbufferSend(mqtt_ring_buf, event->data, event->data_len, portMAX_DELAY);
        if (res != pdTRUE)
            ESP_LOGE(TAG, "Failed to send item\n");
        break;
    }
    case MQTT_EVENT_ERROR:
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_user_task(void *param)
{
    mqtt_obj_t mqtt_obj;
    char *msg_recv = NULL;
    size_t msg_size = 0;
    esp_err_t ret;
    while (1)
    {
        msg_recv = (char *)xRingbufferReceive(mqtt_ring_buf, &msg_size, portMAX_DELAY);
        if (msg_recv)
        {
            msg_recv[msg_size] = '\0';
            ESP_LOGI(TAG, "Payload: %s", msg_recv);
            memset(&mqtt_obj, 0, sizeof(mqtt_obj));
            ret = mqtt_parse_data(msg_recv, &mqtt_obj);
            if (ret == ESP_OK)
            {
                if (mqtt_obj.code == 1)
                {
                    xTimerDelete(ledChangeTimerHandle, 0);
                }
                else
                {
                    uint8_t status;
                    if (!strcmp((char *)mqtt_obj.status, "on"))
                        status = 1;
                    else 
                        status = 0;
                    ESP_LOGI(TAG, "led %d, status %d", mqtt_obj.led, status);
                    gpio_set_level(GPIO_NUM_2, status);
                }
            }
            vRingbufferReturnItem(mqtt_ring_buf, (void *)msg_recv);
        }
    }
}
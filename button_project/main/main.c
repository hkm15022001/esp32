#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "input_iot.h"
#include "dev_uart.h"
#include "string.h"
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define BIT_PRESS_SHORT (1 << 0)
#define BIT_PRESS_NORMAL (1 << 1)
#define BIT_PRESS_LONG (1 << 2)
// static uint8_t s_led_state = 0;
 static EventGroupHandle_t xCreatedEventGroup;

// static void configure_led(void)
//{
//     gpio_reset_pin(BLINK_GPIO);
//     /* Set the GPIO as a push/pull output */
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
// }
//
// static void blink_led(void)
//{
//     /* Set the GPIO level according to the state (LOW or HIGH)*/
//     gpio_set_level(BLINK_GPIO, s_led_state);
// }

void input_event_callback(int pin, uint64_t tick)
{
    // uart_put((uint8_t*)"abc\n",strlen("abc\n"));
    uint64_t pressTime = tick * portTICK_PERIOD_MS;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (pressTime < 1000)
    {   
        xEventGroupSetBitsFromISR(
                              xCreatedEventGroup,   /* The event group being updated. */
                              BIT_PRESS_SHORT, /* The bits being set. */
                              &xHigherPriorityTaskWoken );
        

    } else if(pressTime < 3000){
        xEventGroupSetBitsFromISR(
                              xCreatedEventGroup,   /* The event group being updated. */
                              BIT_PRESS_NORMAL, /* The bits being set. */
                              &xHigherPriorityTaskWoken );
    }
}

void timer_event_callback (){
    xEventGroupSetBits(
                              xCreatedEventGroup,    /* The event group being updated. */
                              BIT_PRESS_LONG );/* The bits being set. */
}

void vTaskCode(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below.
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );*/

    for (;;)
    {
        EventBits_t uxBits = xEventGroupWaitBits(
            xCreatedEventGroup,   /* The event group being tested. */
            BIT_PRESS_SHORT | BIT_PRESS_NORMAL | BIT_PRESS_LONG, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */

        if ((uxBits & BIT_PRESS_SHORT))
        {
            printf("Press Short\n");

        }
        else if ((uxBits & BIT_PRESS_NORMAL))
        {
             printf("Press Normal\n");
        }
        else if ((uxBits & BIT_PRESS_LONG))
        {
           printf("Time out\n");
        };
    }
}

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    // configure_led();

    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    input_io_create(GPIO_NUM_0, ANY_EDGE);
    input_set_callback(input_event_callback);
    timer_set_callback(timer_event_callback);
    xCreatedEventGroup = xEventGroupCreate();
    xTaskCreate(
        vTaskCode, /* Function that implements the task. */
        "NAME",    /* Text name for the task. */
        2048,      /* Stack size in words, not bytes. */
        NULL,      /* Parameter passed into the task. */
        10,        /* Priority at which the task is created. */
        NULL);
}

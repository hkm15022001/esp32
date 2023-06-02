#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "dev_uart.h"
#include "output_iot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";
static TimerHandle_t xTimers;

void vTimerCallback( TimerHandle_t xTimer ) {
    configASSERT( xTimer );
    uart_put((uint8_t*)"đã vào\r\n",strlen("đã vào\r\n"));
    
    uint32_t ulCount;
    ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
    if(ulCount == 0) {
        output_io_toggle(GPIO_NUM_2);
        uart_put((uint8_t*)"toogle\r\n",strlen("toogle\r\n"));
    }
}

void uart_data_cb(uint8_t *data, uint16_t length)
{
    //control led with uart
    if(strstr((char*)data, "ON")){
        uart_put((uint8_t*)"LED ON\r\n",strlen("LED ON\r\n"));
        output_io_set_level(2, 1);
    }
    else if(strstr((char*)data, "OFF")){
        uart_put((uint8_t*)"LED OFF\r\n",strlen("LED OFF\r\n"));
        output_io_set_level(2, 0);
    }

    //set period timer with uart
    char* periodStr = strstr((char*)data,"period:");
    char* period = NULL;
    char* finalPeriod = NULL;

    if(periodStr) {
        period = periodStr+7;
        uart_put((uint8_t*)period,strlen(period));
        finalPeriod =atoi(period);
        xTimers = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "Timer",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     pdMS_TO_TICKS(finalPeriod),
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdTRUE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 0,
                     /* Each timer calls the same callback when
                     it expires. */
                     vTimerCallback
                   );
        output_io_create(GPIO_NUM_2);
        xTimerStart( xTimers, 0 );
    }
    

}

void app_main(void)
{
    uart_set_callback(uart_data_cb);
    uart_init();
    output_io_create(2);
    //uart_put((uint8_t*)"turn LED ON",11);
}

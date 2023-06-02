#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "input_iot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

input_callback_t input_callback = NULL;
timer_callback_t timer_callback = NULL; 
static TimerHandle_t xTimers;

static void IRAM_ATTR gpio_input_handler(void* arg)  {
    int gpio_num = (uint32_t) arg;
    static uint64_t start,stop,tick;
    uint64_t rtc = xTaskGetTickCountFromISR();

    if(gpio_get_level(gpio_num) == 0){
        xTimerStart(xTimers,0);
        start = rtc;
    } else {
        xTimerStop(xTimers,0);
        stop = rtc;
        tick = stop - start;
        input_callback(gpio_num,tick);
    }
}

void vTimerCallback( TimerHandle_t xTimer ) {
    configASSERT( xTimer );
    
    uint32_t ulCount;
    ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
    if(ulCount == 0) {
        timer_callback();
    }
}

void input_io_create(gpio_num_t gpio_num, interrupt_type_edge_t type)
{
    gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void*) gpio_num);  
    xTimers = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "Timer",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     pdMS_TO_TICKS(5000),
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
    
   
}

void input_io_get_level(gpio_num_t gpio_num)
{  
    return gpio_get_level(gpio_num);
}

void input_set_callback(void * cb)
{
    input_callback = cb;
}

void timer_set_callback(void * cb)
{
    timer_callback = cb;
}
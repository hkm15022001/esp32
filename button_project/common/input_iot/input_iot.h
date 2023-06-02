#ifndef INPUT_IO_H
#define INPUT_IO_H
#include "esp_err.h"
#include "hal/gpio_types.h"

typedef void (*input_callback_t) (int,uint64_t);
typedef void (*timer_callback_t) ();

typedef enum{
    LO_TO_HI = 1,
    HI_TO_LO = 2,
    ANY_EDGE = 3
}   interrupt_type_edge_t;

void input_io_create(gpio_num_t gpio_num, interrupt_type_edge_t type);
void input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void * cb);
void timer_set_callback(void * cb);

#endif

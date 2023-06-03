#ifndef _COMMON_USER_H_
#define _COMMON_USER_H_

#include "header.h"
typedef enum
{
    NORMAL = 0,
    SMARTCONFIG,
} gateway_mode_t;

typedef struct 
{  
    uint8_t led;
    uint8_t status[8];
    uint8_t code;
} mqtt_obj_t;

esp_err_t mqtt_parse_data(char *mqtt_data, mqtt_obj_t *mqtt_obj);
#endif
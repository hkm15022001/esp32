#include "common_user.h"
#include "cJSON.h"

esp_err_t mqtt_parse_data(char *mqtt_data, mqtt_obj_t *mqtt_obj)
{
    cJSON *root = cJSON_Parse(mqtt_data);
    if (root == NULL)
        return ESP_FAIL;
    cJSON *elem = NULL;
    cJSON_ArrayForEach(elem, root)
    {
        if (elem->string)
        {
            char *elem_str = elem->string;
            if (!strcmp(elem_str, "led"))
            {
                mqtt_obj->led = elem->valueint;
            }
            else if (!strcmp(elem_str, "status"))
            {
                strcpy((char *)mqtt_obj->status, elem->valuestring);
            }
            else if (!strcmp(elem_str, "code"))
            {
                mqtt_obj->code = elem->valueint;
            }
        }
    }
    return ESP_OK;
}
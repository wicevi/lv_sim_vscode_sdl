#ifndef IR_IMAGING_H
#define IR_IMAGING_H

#include "lvgl.h"

typedef struct
{
    float *temperature_buf;
    float max_temperature;
    float min_temperature;
    uint8_t upate_flag;
    lv_coord_t row_num, col_num;
    lv_coord_t pixel_size;
    lv_coord_t x_offset;
    lv_coord_t y_offset;
} ir_imaging_config_t;

lv_obj_t *ir_imaging_create_main_page(lv_obj_t *parent, void *user_data);

#endif

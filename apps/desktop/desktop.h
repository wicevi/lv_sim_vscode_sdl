#ifndef DESKTOP_H
#define DESKTOP_H

#include "lvgl.h"

typedef struct
{
    const char *name;
    const lv_img_dsc_t *icon_dsc;
    const char *icon_path;
    uint16_t icon_zoom;
    uint8_t position;
    lv_obj_t *(*create_func)(lv_obj_t *parent, void *user_data);
    void *user_data;
} desktop_app_config_t;

typedef struct
{
    const lv_img_dsc_t *bg_img_dsc;
    const char *bg_img_path;
    lv_coord_t item_size;
    lv_color_t item_name_color;
    uint8_t app_num;
    desktop_app_config_t **app_configs;
} desktop_create_config_t;

typedef struct
{
    uint8_t row_num, col_num;
    bool *position_state;
    lv_coord_t item_size;
    lv_coord_t space_size;
} desktop_app_layout_config_t;

lv_obj_t *desktop_create_main_page(lv_obj_t *parent, void *user_data);

#endif
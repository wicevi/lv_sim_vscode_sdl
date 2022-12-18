#ifndef LVGL_TOOL_H
#define LVGL_TOOL_H

#include "lvgl.h"

lv_obj_t *lv_tl_create_base_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);
lv_coord_t lv_tl_obj_get_absolute_x(lv_obj_t *obj, lv_obj_t *scr);
lv_coord_t lv_tl_obj_get_absolute_y(lv_obj_t *obj, lv_obj_t *scr);
void lv_tl_indev_get_absolute_point(lv_point_t *point, lv_obj_t *obj, lv_obj_t *scr);

#endif

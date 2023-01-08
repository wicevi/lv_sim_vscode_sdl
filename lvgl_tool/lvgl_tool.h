#ifndef LVGL_TOOL_H
#define LVGL_TOOL_H

#include "lvgl.h"

typedef enum
{
    MSG_TYPE_DEFAULT = 0,
    MSG_TYPE_SUCCESS,
    MSG_TYPE_WARN,
    MSG_TYPE_ERROR,
    MSG_TYPE_MAX,
} lv_tl_message_type_t;

#define LV_TL_MSG_DEFAULT_BG_COLOR  LV_COLOR_MAKE(233, 233, 235)
#define LV_TL_MSG_DEFAULT_COLOR     LV_COLOR_MAKE(0x90, 0x93, 0x99)
#define LV_TL_MSG_SUCCESS_BG_COLOR  LV_COLOR_MAKE(225, 243, 216)
#define LV_TL_MSG_SUCCESS_COLOR     LV_COLOR_MAKE(0x67, 0xC2, 0x3A)
#define LV_TL_MSG_WARN_BG_COLOR     LV_COLOR_MAKE(250, 236, 216)
#define LV_TL_MSG_WARN_COLOR        LV_COLOR_MAKE(0xE6, 0xA2, 0x3C)
#define LV_TL_MSG_ERROR_BG_COLOR    LV_COLOR_MAKE(253, 226, 226)
#define LV_TL_MSG_ERROR_COLOR       LV_COLOR_MAKE(0xF5, 0x6C, 0x6C)

lv_obj_t *lv_tl_create_base_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);
lv_obj_t *lv_tl_create_flex_linear_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h, lv_flex_flow_t flow, lv_flex_align_t main_place);

lv_coord_t lv_tl_obj_get_absolute_x(lv_obj_t *obj, lv_obj_t *scr);
lv_coord_t lv_tl_obj_get_absolute_y(lv_obj_t *obj, lv_obj_t *scr);
void lv_tl_indev_get_absolute_point(lv_point_t *point, lv_obj_t *obj, lv_obj_t *scr);

lv_color_t lv_tl_get_msg_type_color(lv_tl_message_type_t msg_type);
lv_color_t lv_tl_get_msg_type_bg_color(lv_tl_message_type_t msg_type);

lv_obj_t *lv_tl_create_notification(lv_tl_message_type_t msg_type, uint32_t show_time_ms, const char *msg_fmt, ...);
void lv_tl_destroy_notification(lv_obj_t *notify_layout);

#endif

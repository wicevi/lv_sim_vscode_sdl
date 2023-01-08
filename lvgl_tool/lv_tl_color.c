#include "lvgl_tool.h"

static const lv_color_t msg_color[] = { 
    LV_TL_MSG_DEFAULT_COLOR, 
    LV_TL_MSG_SUCCESS_COLOR, 
    LV_TL_MSG_WARN_COLOR, 
    LV_TL_MSG_ERROR_COLOR 
};

static const lv_color_t msg_bg_color[] = { 
    LV_TL_MSG_DEFAULT_BG_COLOR, 
    LV_TL_MSG_SUCCESS_BG_COLOR, 
    LV_TL_MSG_WARN_BG_COLOR, 
    LV_TL_MSG_ERROR_BG_COLOR 
};

lv_color_t lv_tl_get_msg_type_color(lv_tl_message_type_t msg_type)
{
    return msg_color[msg_type];
}

lv_color_t lv_tl_get_msg_type_bg_color(lv_tl_message_type_t msg_type)
{
    return msg_bg_color[msg_type];
}

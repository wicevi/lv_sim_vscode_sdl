#include "lvgl_tool.h"

lv_obj_t *lv_tl_create_base_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *base_layout = lv_obj_create(parent);
    lv_obj_set_size(base_layout, w, h);
    lv_obj_set_style_pad_all(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(base_layout, LV_OPA_TRANSP, LV_PART_MAIN);
    
    return base_layout;
}



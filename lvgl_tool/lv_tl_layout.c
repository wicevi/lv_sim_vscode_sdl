#include "lvgl_tool.h"

/// @brief 创建个基础布局容器（0边框 0填充 0外距 0圆角 背景透明）
/// @param parent 父容器
/// @param w 宽
/// @param h 高
/// @return 创建的容器
lv_obj_t *lv_tl_create_base_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *base_layout = NULL;
    if (parent == NULL) return NULL;

    base_layout = lv_obj_create(parent);
    lv_obj_set_size(base_layout, w, h);
    lv_obj_set_style_pad_all(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(base_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(base_layout, LV_OPA_TRANSP, LV_PART_MAIN);
    
    return base_layout;
}
/// @brief 创建个弹性不换行布局容器（0边框 0填充 0外距 0圆角 背景透明）
/// @param parent 父容器
/// @param w 宽
/// @param h 高
/// @return 创建的容器
lv_obj_t *lv_tl_create_flex_linear_layout(lv_obj_t *parent, lv_coord_t w, lv_coord_t h, lv_flex_flow_t flow, lv_flex_align_t main_place)
{
    lv_obj_t *flex_linear_layout = NULL;
    if (parent == NULL) return NULL;

    flex_linear_layout = lv_tl_create_base_layout(parent, w, h);
    lv_obj_set_flex_flow(flex_linear_layout, flow);
    lv_obj_set_flex_align(flex_linear_layout, main_place, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    return flex_linear_layout;
}



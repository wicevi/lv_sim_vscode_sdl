#include "lvgl_tool.h"

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

lv_coord_t lv_tl_obj_get_absolute_x(lv_obj_t *obj, lv_obj_t *scr)
{
    lv_coord_t absolute_x = 0;
    lv_obj_t *parent = NULL;
    if (obj == NULL || scr == NULL) return NULL;

    absolute_x = lv_obj_get_x(obj);
    while (1) {
        parent = lv_obj_get_parent(obj);
        if (parent != NULL && parent != scr) {
            absolute_x += lv_obj_get_x(parent);
            obj = parent;
        } else break;
    }
    return absolute_x;
}

lv_coord_t lv_tl_obj_get_absolute_y(lv_obj_t *obj, lv_obj_t *scr)
{
    lv_coord_t absolute_y = 0;
    lv_obj_t *parent = NULL;
    if (obj == NULL || scr == NULL) return NULL;

    absolute_y = lv_obj_get_y(obj);
    while (1) {
        parent = lv_obj_get_parent(obj);
        if (parent != NULL && parent != scr) {
            absolute_y += lv_obj_get_y(parent);
            obj = parent;
        } else break;
    }
    return absolute_y;
}

void lv_tl_indev_get_absolute_point(lv_point_t *point, lv_obj_t *obj, lv_obj_t *scr)
{
    const lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL || point == NULL || obj == NULL || scr == NULL) return;

    lv_indev_get_point(indev, point);
    point->x -= lv_tl_obj_get_absolute_x(obj, scr);
    point->y -= lv_tl_obj_get_absolute_y(obj, scr);
}

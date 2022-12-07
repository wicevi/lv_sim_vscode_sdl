#include "lvgl_tool.h"
#include "../desktop.h"

#define TOP_STATUS_BAR_HEIGHT   20
#define TOP_STATUS_BAR_COLOR    lv_color_hex(0X000000)
#define TOP_STATUS_BAR_BG_OPA   LV_OPA_30

#define TOP_STATUS_LABEL_COLOR  lv_color_hex(0XFFFFFF)

#define APP_ITEM_MIN_SIZE       32
#define APP_ITEM_MIN_SPACE      10
#define APP_ITEM_NAME_COLOR     lv_color_hex(0XFFFFFF)

static lv_obj_t *desktop_global_layout = NULL;
static lv_obj_t *desktop_bg_img = NULL;
static lv_obj_t *top_status_bar = NULL, *top_bar_left_div = NULL, *top_bar_right_div = NULL;
static lv_obj_t *time_label = NULL, *usb_label = NULL, *warn_label = NULL;
static lv_obj_t *bat_label = NULL, *charge_label = NULL, *wifi_label = NULL, *ble_label = NULL;
static lv_obj_t *app_content_layout = NULL;
static lv_style_t app_item_pressed_style;

static void app_item_event_cb(lv_event_t *event)
{
    lv_obj_t *app_item = lv_event_get_target(event);
    lv_indev_t *indev = lv_indev_get_act();
    lv_event_code_t event_code = lv_event_get_code(event);
    desktop_app_config_t *app_config = NULL;
    static lv_point_t pressed_point;
    lv_point_t point, vect;
    lv_coord_t parent_w, parent_h, app_w, app_h;
    lv_coord_t new_x, new_y;
    if (app_item == NULL || indev == NULL) return;
    switch (event_code)
    {
        case LV_EVENT_PRESSED:
            lv_indev_get_point(indev, &pressed_point);
            break;
        case LV_EVENT_PRESSING:
            if (lv_obj_has_flag(app_item, LV_OBJ_FLAG_USER_1)) {
                lv_indev_get_vect(indev, &vect);
                parent_w = lv_obj_get_width(lv_obj_get_parent(app_item));
                parent_h = lv_obj_get_height(lv_obj_get_parent(app_item));
                app_w = lv_obj_get_width(app_item);
                app_h = lv_obj_get_height(app_item);
                new_x = lv_obj_get_x(app_item) + vect.x;
                new_y = lv_obj_get_y(app_item) + vect.y;
                if (new_x > (parent_w - app_w)) new_x = parent_w - app_w;
                else if (new_x < 0) new_x = 0;
                if (new_y > (parent_h - app_h)) new_y = parent_h - app_h;
                else if (new_y < 0) new_y = 0;
                lv_obj_set_pos(app_item, new_x, new_y);
            }
            break;
        case LV_EVENT_SHORT_CLICKED:
            lv_indev_get_point(indev, &point);
            if (LV_ABS(point.x - pressed_point.x) < 10 && LV_ABS(point.y - pressed_point.y) < 10) {
                app_config = (desktop_app_config_t *)lv_event_get_user_data(event);
                if (app_config != NULL && app_config->create_func != NULL) app_config->create_func(app_content_layout, app_config->user_data);
                else {
                    LV_LOG_USER("Application not implemented!");
                }
            }
            break;
        case LV_EVENT_LONG_PRESSED:
            lv_indev_get_point(indev, &point);
            if (LV_ABS(point.x - pressed_point.x) < 10 && LV_ABS(point.y - pressed_point.y) < 10) {
                lv_obj_add_flag(app_item, LV_OBJ_FLAG_USER_1);
                lv_style_set_bg_opa(&app_item_pressed_style, LV_OPA_20);
                lv_obj_report_style_change(&app_item_pressed_style);
            }
            break;
        case LV_EVENT_RELEASED:
            lv_obj_clear_flag(app_item, LV_OBJ_FLAG_USER_1);
            lv_style_set_bg_opa(&app_item_pressed_style, LV_OPA_10);
            lv_obj_report_style_change(&app_item_pressed_style);
            break;
        default:
            break;
    }
}

lv_obj_t *desktop_create_main_page(lv_obj_t *parent, void *user_data)
{
    uint8_t app_row_num = 0, app_col_num = 0;
    lv_coord_t app_space = APP_ITEM_MIN_SPACE;
    lv_coord_t parent_w = LV_SIZE_CONTENT, parent_h = LV_SIZE_CONTENT;
    desktop_create_config_t *desktop_config = NULL;
    
    if (user_data == NULL) return NULL;
    if (parent == NULL) parent = lv_scr_act();
    desktop_config = (desktop_create_config_t *)user_data;

    parent_w = lv_obj_get_width(parent);
    parent_h = lv_obj_get_height(parent);
    
    desktop_global_layout = lv_tl_create_base_layout(parent, parent_w, parent_h);
    lv_obj_center(desktop_global_layout);

    desktop_bg_img = lv_img_create(desktop_global_layout);
    if (desktop_config->bg_img_dsc != NULL) {
        lv_img_set_src(desktop_bg_img, desktop_config->bg_img_dsc);
    } else if (desktop_config->bg_img_path != NULL) {
        lv_img_set_src(desktop_bg_img, desktop_config->bg_img_path);
    } else {
        //TODO: set default bg img

    }
    lv_obj_center(desktop_bg_img);

    top_status_bar = lv_tl_create_base_layout(desktop_global_layout, parent_w, TOP_STATUS_BAR_HEIGHT);
    lv_obj_align(top_status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_status_bar, TOP_STATUS_BAR_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_status_bar, TOP_STATUS_BAR_BG_OPA, LV_PART_MAIN);

    top_bar_left_div = lv_tl_create_base_layout(top_status_bar, LV_SIZE_CONTENT, TOP_STATUS_BAR_HEIGHT);
    lv_obj_align(top_bar_left_div, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_flex_flow(top_bar_left_div, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar_left_div, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(top_bar_left_div, 4, LV_PART_MAIN);

    time_label = lv_label_create(top_bar_left_div);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_style_text_color(time_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    usb_label = lv_label_create(top_bar_left_div);
    lv_label_set_text(usb_label, LV_SYMBOL_USB);
    lv_obj_set_style_text_color(usb_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    warn_label = lv_label_create(top_bar_left_div);
    lv_label_set_text(warn_label, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_color(warn_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    top_bar_right_div = lv_tl_create_base_layout(top_status_bar, LV_SIZE_CONTENT, TOP_STATUS_BAR_HEIGHT);
    lv_obj_align(top_bar_right_div, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_flex_flow(top_bar_right_div, LV_FLEX_FLOW_ROW_REVERSE);
    lv_obj_set_flex_align(top_bar_right_div, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_right(top_bar_right_div, 4, LV_PART_MAIN);

    bat_label = lv_label_create(top_bar_right_div);
    lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_1 " 10%");
    lv_obj_set_style_text_color(bat_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    charge_label = lv_label_create(top_bar_right_div);
    lv_label_set_text(charge_label, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_color(charge_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    wifi_label = lv_label_create(top_bar_right_div);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    ble_label = lv_label_create(top_bar_right_div);
    lv_label_set_text(ble_label, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_color(ble_label, TOP_STATUS_LABEL_COLOR, LV_PART_MAIN);

    lv_obj_t *app_item_layout, *app_icon, *app_name;
    lv_coord_t app_item_x, app_item_y;
    desktop_app_config_t *app_config;
    app_content_layout = lv_tl_create_base_layout(desktop_global_layout, parent_w, parent_h - TOP_STATUS_BAR_HEIGHT);
    lv_obj_align(app_content_layout, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_style_init(&app_item_pressed_style);
    lv_style_set_radius(&app_item_pressed_style, 4);
    lv_style_set_bg_opa(&app_item_pressed_style, LV_OPA_10);
    if (desktop_config->app_num > 0 && desktop_config->app_configs != NULL) {
        if (desktop_config->item_size < APP_ITEM_MIN_SIZE) desktop_config->item_size = APP_ITEM_MIN_SIZE;
        app_row_num = parent_w / (desktop_config->item_size + (APP_ITEM_MIN_SPACE * 4 / 3 + 1));
        app_space = (parent_w - (desktop_config->item_size * app_row_num)) / 4;
        app_col_num = (parent_h - TOP_STATUS_BAR_HEIGHT) / (desktop_config->item_size + (app_space * 4 / 3 + 1));
        LV_LOG_USER("row_num: %d, col_num: %d, space: %d.", app_row_num, app_col_num, app_space);
        for (uint8_t i = 0; i < desktop_config->app_num; i++) {
            app_config = desktop_config->app_configs[i];
            if (app_config != NULL && app_config->position < (app_row_num * app_col_num)) {
                app_item_layout = lv_tl_create_base_layout(app_content_layout, desktop_config->item_size, desktop_config->item_size);
                app_item_x = ((app_config->position - 1) % app_row_num) * (desktop_config->item_size + app_space) + app_space;
                app_item_y = ((app_config->position - 1) / app_row_num) * (desktop_config->item_size + app_space) + app_space;
                lv_obj_set_pos(app_item_layout, app_item_x, app_item_y);
                lv_obj_add_style(app_item_layout, &app_item_pressed_style, LV_STATE_PRESSED);
                lv_obj_add_event_cb(app_item_layout, app_item_event_cb, LV_EVENT_ALL, app_config);
                LV_LOG_USER("position: %d, x: %d, y: %d.", app_config->position, app_item_x, app_item_y);

                app_icon = lv_img_create(app_item_layout);
                if (app_config->icon_dsc != NULL) {
                    lv_img_set_src(app_icon, app_config->icon_dsc);
                } else if (app_config->icon_path != NULL) {
                    lv_img_set_src(app_icon, app_config->icon_path);
                } else {
                    //TODO: set default bg img

                }
                lv_img_set_size_mode(app_icon, LV_IMG_SIZE_MODE_REAL);
                lv_img_set_antialias(app_icon, true);
                lv_img_set_zoom(app_icon, app_config->icon_zoom);
                lv_obj_align(app_icon, LV_ALIGN_TOP_MID, 0, 0);

                app_name = lv_label_create(app_item_layout);
                lv_obj_align(app_name, LV_ALIGN_BOTTOM_MID, 0, -2);
                lv_obj_set_size(app_name, desktop_config->item_size, LV_SIZE_CONTENT);
                lv_label_set_text(app_name, app_config->name);
                lv_label_set_long_mode(app_name, LV_LABEL_LONG_CLIP);
                lv_obj_set_style_text_align(app_name, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
                lv_obj_set_style_text_color(app_name, desktop_config->item_name_color, LV_PART_MAIN);
            }
        }
    }

    return desktop_global_layout;
}


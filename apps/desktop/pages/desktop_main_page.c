#include "lvgl_tool.h"
#include "../desktop.h"

//顶部状态栏相关
#define TOP_STATUS_BAR_HEIGHT       20                          //状态栏高度
#define TOP_STATUS_BAR_COLOR        lv_color_hex(0X000000)      //状态栏背景颜色
#define TOP_STATUS_BAR_BG_OPA       LV_OPA_50                   //状态栏背景透明度
#define TOP_STATUS_LABEL_COLOR      lv_color_hex(0XFFFFFF)      //状态栏图标颜色
//底部菜单栏相关
#define BOTTOM_MENU_BAR_HEIGHT      32                          //菜单栏高度
#define BOTTOM_MENU_BAR_COLOR       lv_color_hex(0X000000)      //菜单栏背景颜色
#define BOTTOM_MENU_BAR_BG_OPA      LV_OPA_50                   //菜单栏背景透明度
#define BOTTOM_MENU_NUM             3                           //菜单项数量
#define BOTTOM_MENU_CLICK_BG_OPA    LV_OPA_20                   //菜单项点击时的背景透明度
#define BOTTOM_MENU_IMG_SIZE        32                          //菜单项图标大小
#define BOTTOM_MENU_IMG_COLOR       lv_color_hex(0XFFFFFF)      //菜单项图标颜色
//APP应用相关
#define APP_ITEM_MIN_SIZE           32          //每个APP应用最小尺寸
#define APP_ITEM_MAX_SIZE           160         //每个APP应用最大尺寸
#define APP_ITEM_MIN_SPACE          10          //APP应用之间最小间隔
#define APP_ITEM_MAX_ROW_NUM        3           //单行APP应用最大数量
#define APP_ITEM_MOVE_BG_OPA        LV_OPA_30   //APP应用移动时背景透明度
#define APP_ITEM_CLICK_BG_OPA       LV_OPA_10   //APP应用点击时背景透明度
#define APP_ITEM_START_MOVE_MIN_T   2           //APP应用可移动最小长按时间（单位长按事件触发间隔时间）
#define APP_ITEM_TOUCH_MIN_DIS      20          //APP两次关联事件偏移最小值
#define APP_CACHE_MAX_NUM           3           //同时最多打开几个APP

/// @brief 全局容器
static lv_obj_t *desktop_global_layout = NULL;
/// @brief 背景图片
static lv_obj_t *desktop_bg_img = NULL;
/// @brief 顶部状态栏
static lv_obj_t *top_status_bar = NULL;
/// @brief 顶部状态栏左侧图标
static lv_obj_t *time_label = NULL, *usb_label = NULL, *warn_label = NULL;
/// @brief 顶部状态栏右侧图标
static lv_obj_t *bat_label = NULL, *charge_label = NULL, *wifi_label = NULL, *ble_label = NULL;
/// @brief APP应用容器
static lv_obj_t *app_content_layout = NULL;
/// @brief APP应用点击时样式
static lv_style_t app_item_pressed_style;
/// @brief 底部菜单栏
static lv_obj_t *bottom_menu_bar = NULL;
/// @brief 菜单项点击时样式
static lv_style_t menu_item_pressed_style;
/// @brief 底部菜单栏各项目图标资源
static const void *menu_img_src[BOTTOM_MENU_NUM] = {
    "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/list.png",
    "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/home.png",
    "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/back.png"
};
/// @brief APP应用布局配置
static desktop_app_layout_config_t app_layout_config = {0};
/// @brief APP应用缓存
static desktop_app_cache_t app_caches[APP_CACHE_MAX_NUM] = {0};
/// @brief 当前活跃APP应用缓存
static desktop_app_cache_t *active_app_cache = NULL;
/// @brief 查找指定创建函数的缓存对象
/// @param func_prt 应用创建函数指针
/// @return 缓存对象指针
static desktop_app_cache_t *desktop_get_app_cache_from_func(void *func_prt)
{
    if (func_prt == NULL) return NULL;

    for (uint8_t i = 0; i < APP_CACHE_MAX_NUM; i++) {
        if (app_caches[i].create_func_prt == func_prt) return &app_caches[i];
    }
    return NULL;
}
/// @brief 获取空闲的应用缓存对象
/// @param 无
/// @return 缓存对象指针
static desktop_app_cache_t *desktop_get_free_app_cache(void)
{
    for (uint8_t i = 0; i < APP_CACHE_MAX_NUM; i++) {
        if (app_caches[i].create_func_prt == NULL && app_caches[i].app_content_layout == NULL) return &app_caches[i];
    }
    return NULL;
}
/// @brief 根据坐标获取应用位置（1~N）
/// @param point 坐标
/// @param app_layout_cfg 布局配置
/// @return 应用位置
static uint8_t desktop_get_app_position(lv_point_t *point, desktop_app_layout_config_t *app_layout_cfg)
{
    lv_coord_t center_x = point->x + (app_layout_cfg->item_size / 2), center_y = point->y + (app_layout_cfg->item_size / 2);
    uint8_t app_row = 0, app_col = 0;

    if (center_x < app_layout_cfg->x_space_size) app_row = 1;
    else {
        center_x -= (app_layout_cfg->x_space_size / 2);
        app_row = center_x / (app_layout_cfg->x_space_size + app_layout_cfg->item_size) + ((center_x % (app_layout_cfg->x_space_size + app_layout_cfg->item_size)) > 0 ? 1 : 0);
    }

    if (center_y < app_layout_cfg->y_space_size) app_col = 1;
    else {
        center_y -= (app_layout_cfg->y_space_size / 2);
        app_col = center_y / (app_layout_cfg->y_space_size + app_layout_cfg->item_size) + ((center_y % (app_layout_cfg->y_space_size + app_layout_cfg->item_size)) > 0 ? 1 : 0);
    }

    return app_layout_cfg->row_num * (app_col - 1) + app_row;
}
/// @brief 根据应用位置获取坐标
/// @param position 应用位置
/// @param point 获取到的坐标
/// @param app_layout_cfg 布局配置
static void desktop_get_app_point(uint8_t position, lv_point_t *point, desktop_app_layout_config_t *app_layout_cfg)
{
    point->x = ((position - 1) % app_layout_cfg->row_num) * (app_layout_cfg->item_size + app_layout_cfg->x_space_size) + app_layout_cfg->x_space_size;
    point->y = ((position - 1) / app_layout_cfg->row_num) * (app_layout_cfg->item_size + app_layout_cfg->y_space_size) + app_layout_cfg->y_space_size;
}
/// @brief 启动对应配置的APP
/// @param app_config APP配置
static void desktop_run_app(desktop_app_config_t *app_config)
{
    lv_color_t bg_color;
    desktop_app_cache_t *app_cache = NULL;
    if (app_config == NULL) return;
    if (app_config->create_func == NULL) {
        //TODO:显示提示弹窗

        LV_LOG_USER("Application not implemented!");
        return;
    }

    app_cache = desktop_get_app_cache_from_func(app_config->create_func);
    if (app_cache != NULL) {
        if (app_cache->app_content_layout != NULL) {
            lv_obj_clear_flag(app_cache->app_content_layout, LV_OBJ_FLAG_HIDDEN);
            bg_color = lv_obj_get_style_bg_color(app_cache->app_content_layout, LV_PART_MAIN);
            lv_obj_set_style_bg_color(desktop_global_layout, bg_color, LV_PART_MAIN);
            lv_obj_add_flag(desktop_bg_img, LV_OBJ_FLAG_HIDDEN);
            active_app_cache = app_cache;
            return;
        } else {
            app_cache->create_func_prt = NULL;
        }
    }

    app_cache = desktop_get_free_app_cache();
    if (app_cache == NULL) {
        //TODO:显示提示弹窗

        LV_LOG_USER("The app cache is full!");
        return;
    }

    app_cache->app_content_layout = app_config->create_func(app_content_layout, app_config->user_data);
    if (app_cache->app_content_layout == NULL) {
        //TODO:显示提示弹窗

        LV_LOG_USER("App startup failed!");
        return;
    }
    bg_color = lv_obj_get_style_bg_color(app_cache->app_content_layout, LV_PART_MAIN);
    lv_obj_set_style_bg_color(desktop_global_layout, bg_color, LV_PART_MAIN);
    lv_obj_add_flag(desktop_bg_img, LV_OBJ_FLAG_HIDDEN);
    //TODO: 注册应用退出事件

    app_cache->create_func_prt = app_config->create_func;
    active_app_cache = app_cache;
}
/// @brief 底部菜单栏列表按钮执行函数
/// @param 无
static void desktop_list_btn_click(void)
{
    //TODO: 显示所有缓存中的应用列表

}
/// @brief 底部菜单栏主页按钮执行函数
/// @param 无
static void desktop_home_btn_click(void)
{
    if (active_app_cache == NULL) return;
    if (active_app_cache->app_content_layout == NULL || active_app_cache->create_func_prt == NULL) {
        active_app_cache->app_content_layout = NULL;
        active_app_cache->create_func_prt = NULL;
        active_app_cache = NULL;
        return;
    }
    lv_obj_clear_flag(desktop_bg_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(active_app_cache->app_content_layout, LV_OBJ_FLAG_HIDDEN);
    active_app_cache = NULL;
}
/// @brief 底部菜单栏返回按钮执行函数
/// @param 无
static void desktop_back_btn_click(void)
{
    if (active_app_cache == NULL) return;
    if (active_app_cache->app_content_layout == NULL || active_app_cache->create_func_prt == NULL) {
        active_app_cache->app_content_layout = NULL;
        active_app_cache->create_func_prt = NULL;
        active_app_cache = NULL;
        return;
    }
    //TODO: 发送事件给应用

}
/// @brief 底部菜单栏点击事件
/// @param event 事件对象
static void desktop_menu_click_event_cb(lv_event_t *event)
{
    void *menu_img = lv_event_get_user_data(event);
    if (menu_img == menu_img_src[0]) {
        desktop_list_btn_click();
    } else if (menu_img == menu_img_src[1]) {
        desktop_home_btn_click();
    } else if (menu_img == menu_img_src[2]) {
        desktop_back_btn_click();
    }
}
/// @brief APP应用事件回调函数
/// @param event 事件对象
static void desktop_app_item_event_cb(lv_event_t *event)
{
    static lv_point_t pressed_point;
    static uint8_t long_pressed_times, long_pressed_move;
    uint8_t new_position;
    lv_obj_t *app_item = lv_event_get_target(event);
    lv_indev_t *indev = lv_indev_get_act();
    lv_event_code_t event_code = lv_event_get_code(event);
    desktop_app_config_t *app_config = NULL;
    lv_point_t point, vect;
    lv_coord_t parent_w, parent_h, app_w, app_h;
    lv_coord_t new_x, new_y;
    if (app_item == NULL || indev == NULL) return;
    switch (event_code)
    {
        case LV_EVENT_PRESSED:
            lv_indev_get_point(indev, &pressed_point);
            long_pressed_times = 0;
            long_pressed_move = 0;
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
                if (new_y > (parent_h - app_h - app_layout_config.height_offset)) new_y = parent_h - app_h - app_layout_config.height_offset;
                else if (new_y < 0) new_y = 0;
                lv_obj_set_pos(app_item, new_x, new_y);
            }
            break;
        case LV_EVENT_SHORT_CLICKED:
            lv_indev_get_point(indev, &point);
            if (LV_ABS(point.x - pressed_point.x) < APP_ITEM_TOUCH_MIN_DIS && LV_ABS(point.y - pressed_point.y) < APP_ITEM_TOUCH_MIN_DIS) {
                app_config = (desktop_app_config_t *)lv_event_get_user_data(event);
                desktop_run_app(app_config);
            }
            break;
        case LV_EVENT_LONG_PRESSED_REPEAT:
            lv_indev_get_point(indev, &point);
            if (LV_ABS(point.x - pressed_point.x) < APP_ITEM_TOUCH_MIN_DIS && LV_ABS(point.y - pressed_point.y) < APP_ITEM_TOUCH_MIN_DIS) {
                long_pressed_times++;
                if (long_pressed_times >= APP_ITEM_START_MOVE_MIN_T && long_pressed_move == 0) {
                    lv_obj_add_flag(app_item, LV_OBJ_FLAG_USER_1);
                    lv_style_set_bg_opa(&app_item_pressed_style, APP_ITEM_MOVE_BG_OPA);
                    lv_obj_report_style_change(&app_item_pressed_style);
                }
            } else {
                long_pressed_move = 1;
            }
            break;
        case LV_EVENT_RELEASED:
            if (lv_obj_has_flag(app_item, LV_OBJ_FLAG_USER_1)) {
                app_config = (desktop_app_config_t *)lv_event_get_user_data(event);
                point.x = lv_obj_get_x(app_item);
                point.y = lv_obj_get_y(app_item);
                new_position = desktop_get_app_position(&point, &app_layout_config);
                if (new_position <= app_layout_config.position_num) {
                    if (app_layout_config.position_state[new_position - 1]) new_position = app_config->position;
                    app_layout_config.position_state[app_config->position - 1] = false;
                    app_layout_config.position_state[new_position - 1] = true;
                    app_config->position = new_position;
                } else new_position = app_config->position;

                desktop_get_app_point(new_position, &point, &app_layout_config);
                lv_obj_set_pos(app_item, point.x, point.y);

                lv_obj_clear_flag(app_item, LV_OBJ_FLAG_USER_1);
                lv_style_set_bg_opa(&app_item_pressed_style, APP_ITEM_CLICK_BG_OPA);
                lv_obj_report_style_change(&app_item_pressed_style);
            }
            break;
        default:
            break;
    }
}
/// @brief 创建桌面应用并返回全局容器对象
/// @param parent 父布局对象
/// @param user_data 应用的配置信息
/// @return 应用的全局容器对象
lv_obj_t *desktop_create_main_page(lv_obj_t *parent, void *user_data)
{
    uint8_t app_row_num = 0, app_col_num = 0;
    lv_coord_t app_space = APP_ITEM_MIN_SPACE, y_space_offset = 0;
    lv_coord_t parent_w = LV_SIZE_CONTENT, parent_h = LV_SIZE_CONTENT;
    desktop_create_config_t *desktop_config = NULL;
    
    if (user_data == NULL || desktop_global_layout != NULL) return NULL;
    if (parent == NULL) parent = lv_scr_act();

    desktop_config = (desktop_create_config_t *)user_data;
    //TODO:  检查desktop_config是否合法

    parent_w = lv_obj_get_width(parent);
    parent_h = lv_obj_get_height(parent);
    
    desktop_global_layout = lv_tl_create_base_layout(parent, parent_w, parent_h);
    lv_obj_set_style_bg_opa(desktop_global_layout, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_center(desktop_global_layout);

    desktop_bg_img = lv_img_create(desktop_global_layout);
    if (desktop_config->bg_img_dsc != NULL) {
        lv_img_set_src(desktop_bg_img, desktop_config->bg_img_dsc);
    } else if (desktop_config->bg_img_path != NULL) {
        lv_img_set_src(desktop_bg_img, desktop_config->bg_img_path);
    } else {
        //TODO: 设置默认背景图片
        
    }
    lv_obj_center(desktop_bg_img);

    lv_obj_t *top_bar_left_div = NULL, *top_bar_right_div = NULL;
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
    lv_point_t app_point;
    lv_coord_t app_layout_height = parent_h - TOP_STATUS_BAR_HEIGHT - BOTTOM_MENU_BAR_HEIGHT;
    desktop_app_config_t *app_config;
    app_content_layout = lv_tl_create_base_layout(desktop_global_layout, parent_w, app_layout_height);
    lv_obj_align(app_content_layout, LV_ALIGN_TOP_MID, 0, TOP_STATUS_BAR_HEIGHT);
    lv_style_init(&app_item_pressed_style);
    lv_style_set_radius(&app_item_pressed_style, 4);
    lv_style_set_bg_opa(&app_item_pressed_style, APP_ITEM_CLICK_BG_OPA);
    if (desktop_config->app_num > 0 && desktop_config->row_num > 0 && desktop_config->app_configs != NULL) {
        if (desktop_config->item_size < APP_ITEM_MIN_SIZE) desktop_config->item_size = APP_ITEM_MIN_SIZE;
        else if (desktop_config->item_size > APP_ITEM_MAX_SIZE) desktop_config->item_size = APP_ITEM_MAX_SIZE;
        if (desktop_config->row_num > APP_ITEM_MAX_ROW_NUM) desktop_config->row_num = APP_ITEM_MAX_ROW_NUM;
        app_row_num = desktop_config->row_num;
        do {
            app_space = (parent_w - (desktop_config->item_size * app_row_num)) / (app_row_num + 1);
            if (app_space < APP_ITEM_MIN_SPACE) app_row_num--;
            else break;
        } while (app_row_num);
        app_col_num = 0;
        do {
            app_col_num++;
        } while ((desktop_config->item_size * app_col_num) + (app_space * (app_col_num + 1)) <= (app_layout_height));
        if (app_col_num) app_col_num--;
        if (app_col_num) {
            y_space_offset = (app_layout_height) - ((desktop_config->item_size * app_col_num) + (app_space * (app_col_num + 1)));
            y_space_offset = y_space_offset / (app_col_num + 1);
            if (desktop_config->y_space_offset > y_space_offset) desktop_config->y_space_offset = y_space_offset;
            if (desktop_config->is_auto_y_offset == 0) y_space_offset = desktop_config->y_space_offset;
        }
        app_layout_config.x_space_size = app_space;
        app_layout_config.y_space_size = app_space + y_space_offset;
        app_layout_config.height_offset = app_layout_height - ((desktop_config->item_size * app_col_num) + (app_layout_config.y_space_size * (app_col_num + 1)));
        LV_LOG_USER("x_space_size: %d, y_space_size: %d height_offset: %d.", app_layout_config.x_space_size, app_layout_config.y_space_size, app_layout_config.height_offset);
        app_layout_config.row_num = app_row_num;
        app_layout_config.position_num = app_row_num * app_col_num;
        app_layout_config.item_size = desktop_config->item_size;
        LV_LOG_USER("position_num: %d.", app_layout_config.position_num);
        app_layout_config.position_state = (bool *)lv_mem_alloc(sizeof(bool) * (app_layout_config.position_num));
        LV_ASSERT(app_layout_config.position_state);
        lv_memset_00(app_layout_config.position_state, sizeof(bool) * (app_layout_config.position_num));
        
        for (uint8_t i = 0; i < desktop_config->app_num; i++) {
            app_config = desktop_config->app_configs[i];
            if (app_config != NULL && app_config->position <= app_layout_config.position_num && app_layout_config.position_state[app_config->position - 1] == false) {
                app_item_layout = lv_tl_create_base_layout(app_content_layout, desktop_config->item_size, desktop_config->item_size);
                desktop_get_app_point(app_config->position, &app_point, &app_layout_config);
                lv_obj_set_pos(app_item_layout, app_point.x, app_point.y);
                lv_obj_add_style(app_item_layout, &app_item_pressed_style, LV_STATE_PRESSED);
                lv_obj_add_event_cb(app_item_layout, desktop_app_item_event_cb, LV_EVENT_ALL, app_config);
                app_layout_config.position_state[app_config->position - 1] = true;
                LV_LOG_USER("position: %d, x: %d, y: %d.", app_config->position, app_point.x, app_point.y);

                app_icon = lv_img_create(app_item_layout);
                if (app_config->icon_dsc != NULL) {
                    lv_img_set_src(app_icon, app_config->icon_dsc);
                } else if (app_config->icon_path != NULL) {
                    lv_img_set_src(app_icon, app_config->icon_path);
                } else {
                    //TODO: 设置默认无图图标

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

    bottom_menu_bar = lv_tl_create_base_layout(desktop_global_layout, parent_w, BOTTOM_MENU_BAR_HEIGHT);
    lv_obj_align(bottom_menu_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottom_menu_bar, BOTTOM_MENU_BAR_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bottom_menu_bar, BOTTOM_MENU_BAR_BG_OPA, LV_PART_MAIN);
    lv_obj_set_flex_flow(bottom_menu_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_menu_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *bottom_menus[BOTTOM_MENU_NUM] = {0};
    lv_obj_t *menu_img = NULL;
    lv_style_init(&menu_item_pressed_style);
    lv_style_set_radius(&menu_item_pressed_style, 12);
    lv_style_set_bg_opa(&menu_item_pressed_style, BOTTOM_MENU_CLICK_BG_OPA);
    for (uint8_t i = 0; i < BOTTOM_MENU_NUM; i++) {
        bottom_menus[i] = lv_tl_create_base_layout(bottom_menu_bar, parent_w / BOTTOM_MENU_NUM, BOTTOM_MENU_BAR_HEIGHT);
        lv_obj_add_style(bottom_menus[i], &menu_item_pressed_style, LV_STATE_PRESSED);
        lv_obj_add_event_cb(bottom_menus[i], desktop_menu_click_event_cb, LV_EVENT_CLICKED, menu_img_src[i]);

        menu_img = lv_img_create(bottom_menus[i]);
        if (menu_img_src[i] == NULL) {
            //TODO: 设置无图图片

        } else {
            lv_img_set_src(menu_img, menu_img_src[i]);
        }
        lv_img_set_size_mode(menu_img, LV_IMG_SIZE_MODE_REAL);
        lv_img_set_antialias(menu_img, true);
        lv_img_set_zoom(menu_img, BOTTOM_MENU_BAR_HEIGHT / 4 * 3 * 255 / BOTTOM_MENU_IMG_SIZE);   
        lv_obj_center(menu_img);
    }
    
    
    return desktop_global_layout;
}


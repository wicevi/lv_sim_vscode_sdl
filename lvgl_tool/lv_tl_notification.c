#include <stdarg.h>
#include "lvgl_tool.h"

#define NOTIFICATION_ICON_DEFAULT_SIZE          (64)
#define NOTIFICATION_ICON_OFFSET_SIZE           (4)

#define NOTIFICATION_LAYOUT_Y_OFFSET            26
#define NOTIFICATION_LAYOUT_ALIGN               LV_ALIGN_TOP_MID
#define NOTIFICATION_LAYOUT_WIDTH_PCT           80
#define NOTIFICATION_LAYOUT_HEIGHT              LV_SIZE_CONTENT
#define NOTIFICATION_LAYOUT_PADDING_SIZE        (6)
#define NOTIFICATION_LAYOUT_RADIUS              (8)
#define NOTIFICATION_MSG_MAX_SIZE               (1024)
#define NOTIFICATION_MSG_LINE_SPACE             (-4)
#define NOTIFICATION_MSG_PADDING_LEFT           (0)
#define NOTIFICATION_MSG_PADDING_TOP            (0)
#define NOTIFICATION_TYPE_ICON_SIZE             (23)
#define NOTIFICATION_CLOSE_ICON_SIZE            (21)
#define NOTIFICATION_CLOSE_ICON_COLOR           LV_COLOR_MAKE(0xAE, 0xAE, 0xAE)
#define NOTIFICATION_ANIM_SHOW_TIME_MS          (500)
#define NOTIFICATION_ANIM_DELAY_TIME_MS         (3000)
#define NOTIFICATION_ANIM_BACK_TIME_MS          (800)
// LV_FONT_DECLARE(puhui_55_14_symbols);
#define NOTIFICATION_MSG_USE_FONT               &lv_font_montserrat_18

static lv_color_t msg_close_icon_color = LV_COLOR_MAKE(0xAE, 0xAE, 0xAE);
static const char *msg_close_icon_src = "A:/home/wicevi/lv_sim_vscode_sdl/lvgl_tool/imgs/close.png";
static const char *msg_type_icon_src[MSG_TYPE_MAX] = {
    "A:/home/wicevi/lv_sim_vscode_sdl/lvgl_tool/imgs/info.png",
    "A:/home/wicevi/lv_sim_vscode_sdl/lvgl_tool/imgs/success.png",
    "A:/home/wicevi/lv_sim_vscode_sdl/lvgl_tool/imgs/warning.png",
    "A:/home/wicevi/lv_sim_vscode_sdl/lvgl_tool/imgs/error.png",
};
/// @brief 获取指定消息类型的图标资源
/// @param msg_type 消息类型
/// @return 图标资源
static const char *lv_tl_get_msg_type_icon_src(lv_tl_message_type_t msg_type)
{
    return msg_type_icon_src[msg_type];
}
/// @brief Y轴动画函数
/// @param var 执行对象
/// @param v 新值
static void notify_anim_exec_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}
/// @brief 动画准备就绪时调用的回调函数
/// @param anim 动画对象
static void notify_anim_ready_cb(lv_anim_t *anim)
{
    lv_obj_t *notify_layout = lv_anim_get_user_data(anim);
    lv_tl_destroy_notification(notify_layout);
}
/// @brief 弹窗删除事件回调函数
/// @param event 事件对象
static void notify_destroy_event_cb(lv_event_t *event)
{
    lv_obj_t *notify_layout = lv_event_get_target(event);
    lv_anim_t *notify_anim = (lv_anim_t *)lv_event_get_user_data(event);
    if (notify_layout != NULL) {
        lv_anim_del(notify_layout, notify_anim_exec_y_cb);
    }
    if (notify_anim != NULL) {
        lv_mem_free(notify_anim);
        LV_LOG_USER("notify_destroy_event_cb");
    }
}
/// @brief 弹窗关闭按钮点击事件回调函数
/// @param event 事件对象
static void notify_close_btn_event_cb(lv_event_t *event)
{
    lv_obj_t *notify_layout = (lv_obj_t *)lv_event_get_user_data(event);
    lv_tl_destroy_notification(notify_layout);
}
/// @brief 创建一条消息提示
/// @param msg_type 消息类型
/// @param show_time_ms 显示时间（若为0则永不消失）
/// @param msg_fmt 消息格式字符串
/// @param 可变参数
/// @return 提示消息的容器对象
lv_obj_t *lv_tl_create_notification(lv_tl_message_type_t msg_type, uint32_t show_time_ms, const char *msg_fmt, ...)
{
    char *msg_buf = NULL;
    va_list msg_args;
    lv_obj_t *notify_layout = NULL, *tmp_layout = NULL, *tmp_obj = NULL;
    lv_anim_t *notify_anim = NULL;
    lv_coord_t notify_layout_w = lv_obj_get_width(lv_layer_top()) * NOTIFICATION_LAYOUT_WIDTH_PCT / 100;
    if (msg_fmt == NULL || notify_layout_w < (NOTIFICATION_TYPE_ICON_SIZE + NOTIFICATION_CLOSE_ICON_SIZE + 4 * NOTIFICATION_LAYOUT_PADDING_SIZE)) return NULL;
    
    msg_buf = (char *)lv_mem_alloc(NOTIFICATION_MSG_MAX_SIZE);
    if (msg_buf == NULL) return NULL;

    notify_anim = (lv_anim_t *)lv_mem_alloc(sizeof(lv_anim_t));
    if (notify_anim == NULL) {
        lv_mem_free(msg_buf);
        return NULL;
    }

    va_start(msg_args, msg_fmt);
    lv_vsnprintf(msg_buf, NOTIFICATION_MSG_MAX_SIZE, msg_fmt, msg_args);
    va_end(msg_args);

    notify_layout = lv_tl_create_flex_linear_layout(lv_layer_top(), notify_layout_w, NOTIFICATION_LAYOUT_HEIGHT, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_obj_align(notify_layout, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(notify_layout, NOTIFICATION_LAYOUT_PADDING_SIZE, LV_PART_MAIN);
    lv_obj_set_style_radius(notify_layout, NOTIFICATION_LAYOUT_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_bg_color(notify_layout, lv_tl_get_msg_type_bg_color(msg_type), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(notify_layout, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(notify_layout, notify_destroy_event_cb, LV_EVENT_DELETE, notify_anim);

    tmp_layout = lv_tl_create_base_layout(notify_layout, NOTIFICATION_TYPE_ICON_SIZE, NOTIFICATION_TYPE_ICON_SIZE);

    tmp_obj = lv_img_create(tmp_layout);
    lv_img_set_src(tmp_obj, lv_tl_get_msg_type_icon_src(msg_type));
    lv_img_set_size_mode(tmp_obj, LV_IMG_SIZE_MODE_REAL);
    lv_img_set_antialias(tmp_obj, true);
    lv_img_set_zoom(tmp_obj, LV_IMG_ZOOM_NONE * (NOTIFICATION_TYPE_ICON_SIZE - NOTIFICATION_ICON_OFFSET_SIZE) / NOTIFICATION_ICON_DEFAULT_SIZE);
    lv_obj_set_size(tmp_obj, NOTIFICATION_TYPE_ICON_SIZE, NOTIFICATION_TYPE_ICON_SIZE);
    lv_obj_align(tmp_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(tmp_obj, lv_tl_get_msg_type_color(msg_type), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tmp_obj, LV_OPA_COVER, LV_PART_MAIN);

    tmp_obj = lv_label_create(notify_layout);
    lv_label_set_text(tmp_obj, msg_buf);
    lv_label_set_long_mode(tmp_obj, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(tmp_obj, notify_layout_w - (NOTIFICATION_TYPE_ICON_SIZE + NOTIFICATION_CLOSE_ICON_SIZE + 3 * NOTIFICATION_LAYOUT_PADDING_SIZE), LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(tmp_obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(tmp_obj, NOTIFICATION_MSG_LINE_SPACE, LV_PART_MAIN);
    lv_obj_set_style_text_color(tmp_obj, lv_tl_get_msg_type_color(msg_type), LV_PART_MAIN);
    lv_obj_set_style_text_font(tmp_obj, NOTIFICATION_MSG_USE_FONT, LV_PART_MAIN);
    lv_obj_set_style_pad_left(tmp_obj, NOTIFICATION_MSG_PADDING_LEFT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(tmp_obj, NOTIFICATION_MSG_PADDING_TOP, LV_PART_MAIN);

    tmp_layout = lv_tl_create_base_layout(notify_layout, NOTIFICATION_CLOSE_ICON_SIZE, NOTIFICATION_CLOSE_ICON_SIZE);
    lv_obj_add_event_cb(tmp_layout, notify_close_btn_event_cb, LV_EVENT_CLICKED, notify_layout);
    lv_obj_set_ext_click_area(tmp_layout, NOTIFICATION_LAYOUT_PADDING_SIZE);
    lv_obj_set_style_radius(tmp_layout, 999, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(tmp_layout, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(tmp_layout, LV_OPA_10, LV_STATE_PRESSED);

    tmp_obj = lv_img_create(tmp_layout);
    lv_img_set_src(tmp_obj, msg_close_icon_src);
    lv_img_set_antialias(tmp_obj, true);
    lv_img_set_zoom(tmp_obj, LV_IMG_ZOOM_NONE * (NOTIFICATION_CLOSE_ICON_SIZE - NOTIFICATION_ICON_OFFSET_SIZE)  / NOTIFICATION_ICON_DEFAULT_SIZE);
    lv_img_set_size_mode(tmp_obj, LV_IMG_SIZE_MODE_REAL);
    lv_obj_set_size(tmp_obj, NOTIFICATION_CLOSE_ICON_SIZE, NOTIFICATION_CLOSE_ICON_SIZE);
    lv_obj_set_style_img_recolor(tmp_obj, msg_close_icon_color, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(tmp_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(tmp_obj, LV_ALIGN_CENTER, 0, 0);

    lv_anim_init(notify_anim);
    lv_anim_set_var(notify_anim, notify_layout);
    lv_anim_set_time(notify_anim, NOTIFICATION_ANIM_SHOW_TIME_MS);
    lv_anim_set_exec_cb(notify_anim, notify_anim_exec_y_cb);
    lv_anim_set_path_cb(notify_anim, lv_anim_path_overshoot);
    lv_anim_set_values(notify_anim, -lv_obj_get_height(notify_layout), NOTIFICATION_LAYOUT_Y_OFFSET);
    lv_anim_set_playback_time(notify_anim, NOTIFICATION_ANIM_BACK_TIME_MS);
    lv_anim_set_playback_delay(notify_anim, NOTIFICATION_ANIM_DELAY_TIME_MS);
    lv_anim_set_ready_cb(notify_anim, notify_anim_ready_cb);
    lv_anim_set_user_data(notify_anim, notify_layout);

    lv_anim_start(notify_anim);
    
    if (msg_buf != NULL) lv_mem_free(msg_buf);
    return notify_layout;
}
/// @brief 销毁一条消息提示
/// @param notify_layout 提示消息的容器对象
void lv_tl_destroy_notification(lv_obj_t *notify_layout)
{
    if (notify_layout == NULL) return;
    lv_obj_del(notify_layout);
}



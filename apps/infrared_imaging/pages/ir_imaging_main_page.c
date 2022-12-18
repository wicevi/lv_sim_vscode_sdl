#include <math.h>
#include "lvgl_tool.h"
#include "../infrared_imaging.h"

//各个颜色值
#define IR_IMG_SCALE_300_COLOR  LV_COLOR_MAKE(0xff, 0x00, 0x00)     //300度颜色
#define IR_IMG_SCALE_50_COLOR   LV_COLOR_MAKE(0xf7, 0x94, 0x1e)     //50度颜色
#define IR_IMG_SCALE_20_COLOR   LV_COLOR_MAKE(0x00, 0x2a, 0x88)     //20度颜色
#define IR_IMG_SCALE_N40_COLOR  LV_COLOR_MAKE(0x00, 0xcd, 0xe9)     //-40度颜色
#define IR_IMG_GLOBAL_BG_COLOR  LV_COLOR_MAKE(0xe6, 0xe6, 0xe6)     //背景颜色
#define IR_IMG_CANVAS_BG_COLOR  LV_COLOR_MAKE(0xF6, 0xF6, 0xF6)     //画布背景颜色
//画布内容相关参数
#define IR_IMG_BORDER_SIZE      (5)     //边框大小
#define IR_IMG_CANVAS_WIDTH     (240)   //画布总宽度
#define IR_IMG_CANVAS_HEIGHT    (144 + IR_IMG_BORDER_SIZE * 2)      //画布总高度
#define IR_IMG_BUF_WIDTH        (192)   //成像区域宽度度
#define IR_IMG_BUF_HEIGHT       (144)   //成像区域高度
#define IR_IMG_LABEL_WIDTH      (36)    //刻度温度值宽度
#define IR_IMG_LABEL_HEIGHT     (14)    //刻度温度值高度
#define IR_IMG_LABEL_X          (IR_IMG_CANVAS_WIDTH - IR_IMG_LABEL_WIDTH - (IR_IMG_CANVAS_WIDTH - IR_IMG_BUF_WIDTH - IR_IMG_BORDER_SIZE - IR_IMG_LABEL_WIDTH) / 2)     //刻度温度值X坐标
#define IR_IMG_LABEL_Y1         (IR_IMG_SCALE_Y - 16)                           //刻度温度值Y坐标1
#define IR_IMG_LABEL_Y2         (IR_IMG_SCALE_Y + IR_IMG_SCALE_HEIGHT - 3)      //刻度温度值Y坐标2
#define IR_IMG_SCALE_WIDTH      (24)                                            //刻度颜色参照图宽度
#define IR_IMG_SCALE_HEIGHT     (IR_IMG_BUF_HEIGHT - IR_IMG_LABEL_HEIGHT * 2)   //刻度颜色参照图高度
#define IR_IMG_SCALE_X          (IR_IMG_LABEL_X + (IR_IMG_LABEL_WIDTH - IR_IMG_SCALE_WIDTH) / 2)    //刻度颜色参照图X坐标
#define IR_IMG_SCALE_Y          (IR_IMG_BORDER_SIZE + IR_IMG_LABEL_HEIGHT)      //刻度颜色参照图Y坐标
//数据信息刷新时间
#define IR_IMG_REFRESH_TIME     10                        
//使用字体声明注册
LV_FONT_DECLARE(puhui_55_10);
LV_FONT_DECLARE(puhui_55_11);
LV_FONT_DECLARE(puhui_55_12);
/// @brief 全局容器
static lv_obj_t *ir_imaging_global_layout = NULL;
/// @brief 全局容器背景颜色
static const lv_color_t global_bg_color = IR_IMG_GLOBAL_BG_COLOR;
/// @brief 数据信息刷新定时器
static lv_timer_t *buf_info_refresh_timer = NULL;
/// @brief 绘制成像图片画布
static lv_obj_t *ir_img_canvas = NULL;
/// @brief 画布颜色缓存
static lv_color_t ir_img_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(IR_IMG_CANVAS_WIDTH, IR_IMG_CANVAS_HEIGHT)];
/// @brief 画布背景颜色
static const lv_color_t canvas_bg_color = IR_IMG_CANVAS_BG_COLOR;
/// @brief 刻度颜色值
static const lv_color_t scale_color[] = { 
    IR_IMG_SCALE_300_COLOR, 
    IR_IMG_SCALE_50_COLOR, 
    IR_IMG_SCALE_20_COLOR, 
    IR_IMG_SCALE_N40_COLOR 
};
/// @brief 刻度颜色值对应温度值
static const int32_t scale_temperature[] = { 
    300, 50, 20, -40 
};
/// @brief 主要温度信息
static lv_obj_t *max_temp_label = NULL, *target_temp_label = NULL, *min_temp_label = NULL;
/// @brief 目标坐标信息
static lv_point_t target_point = {0};
/// @brief 控制按钮容器
static lv_obj_t *ctrl_btn_layout = NULL;
/// @brief 断连相关容器
static lv_obj_t *discnt_show_layout = NULL;
/// @brief 成像图片布局配置
static ir_imaging_buf_layout_config_t buf_layout_config = {0};
/// @brief 根据温度判断属于哪两个刻度区间
/// @param temperature 温度值
/// @param i1 上刻度索引
/// @param i2 下刻度索引
static void ir_imaging_get_scale_color_index(float temperature, uint8_t *i1, uint8_t *i2)
{
    if (temperature > scale_temperature[0]) {
        *i1 = 0;
        *i2 = 0;
    } else if (temperature > scale_temperature[1]) {
        *i1 = 0;
        *i2 = 1;
    } else if (temperature > scale_temperature[2]) {
        *i1 = 1;
        *i2 = 2;
    } else if (temperature > scale_temperature[3]) {
        *i1 = 2;
        *i2 = 3;
    } else {
        *i1 = 3;
        *i2 = 3;
    }
}
/// @brief 在两个颜色值的渐变区间，获取指定分辨率和深度的颜色值
/// @param start_color 起始颜色
/// @param end_color 结束颜色
/// @param all_size 分辨率
/// @param index 深度
/// @return 颜色值
static lv_color_t ir_imaging_get_grad_color(lv_color_t start_color, lv_color_t end_color, int32_t all_size, int32_t index)
{
    int32_t start_r = LV_COLOR_GET_R8(start_color), end_r = LV_COLOR_GET_R8(end_color);
    int32_t start_g = LV_COLOR_GET_G8(start_color), end_g = LV_COLOR_GET_G8(end_color);
    int32_t start_b = LV_COLOR_GET_B8(start_color), end_b = LV_COLOR_GET_B8(end_color);
    int32_t r, g, b;

    r = (int32_t)round((double)index * (double)(end_r - start_r) / (double)all_size) + start_r;
    g = (int32_t)round((double)index * (double)(end_g - start_g) / (double)all_size) + start_g;
    b = (int32_t)round((double)index * (double)(end_b - start_b) / (double)all_size) + start_b;

    if (r > 255) r = 255;
    else if (r < 0) r = 0;
    if (g > 255) g = 255;
    else if (g < 0) g = 0;
    if (b > 255) b = 255;
    else if (b < 0) b = 0;

    return lv_color_make(r, g, b);
}
/// @brief 获取指定温度的颜色值
/// @param temperature 温度值
/// @return 颜色值
static lv_color_t ir_imaging_get_temperature_color(float temperature)
{
    uint8_t ci1, ci2;
    int32_t all_size = 0, index = 0;

    ir_imaging_get_scale_color_index(temperature, &ci1, &ci2);
    if (ci1 == ci2) return scale_color[ci1];
    else if (ci1 == 0) {
        all_size = 2500;
        index = (int32_t)((temperature - 50.0f) / 0.1f);
    } else if (ci1 == 1) {
        all_size = 3000;
        index = (int32_t)((temperature - 20.0f) / 0.01f);
    } else if (ci1 == 2) {
        all_size = 6000;
        index = (int32_t)((temperature + 40.0f) / 0.01f);
    }

    return ir_imaging_get_grad_color(scale_color[ci2], scale_color[ci1], all_size, index);
}
/// @brief 获取指定坐标的温度索引
/// @param point 坐标信息
/// @param layout_cfg 温度成像布局配置
/// @return 索引（若小于0，该点为无效位置）
static int ir_imaging_get_temperature_index(lv_point_t *point, ir_imaging_buf_layout_config_t *layout_cfg)
{
    int index = 0;
    lv_coord_t relative_x = 0;
    lv_coord_t relative_y = 0;
    if (point == NULL || layout_cfg == NULL) return -1;
    relative_x = point->x;
    relative_y = point->y;
    if (relative_x < IR_IMG_BORDER_SIZE + layout_cfg->x_offset || relative_y < IR_IMG_BORDER_SIZE + layout_cfg->y_offset ||
        relative_x > IR_IMG_BORDER_SIZE + layout_cfg->x_offset + layout_cfg->pixel_size * layout_cfg->row_num || 
        relative_y > IR_IMG_BORDER_SIZE + layout_cfg->y_offset + layout_cfg->pixel_size * layout_cfg->col_num) return -2;

    index = (relative_y - (IR_IMG_BORDER_SIZE + layout_cfg->y_offset)) / layout_cfg->pixel_size * layout_cfg->row_num;
    index += (relative_x - (IR_IMG_BORDER_SIZE + layout_cfg->x_offset)) / layout_cfg->pixel_size;
    return index;
}
/// @brief 绘制画布的温度颜色参照图
/// @param max_temperature 最大温度值
/// @param min_temperature 最小温度值
static void ir_imaging_draw_scale(ir_imaging_buf_info_t *buf_info)
{
    char tmp_buf[32] = {0};
    uint8_t color_index[4] = {0};
    uint8_t grad_num = 0, i = 0;
    lv_coord_t grad_item_height = 0;
    lv_draw_rect_dsc_t scale_dsc;
    lv_draw_label_dsc_t label_dsc;

    if (ir_img_canvas == NULL || buf_info == NULL) return;
    if (buf_info->max_temperature < buf_info->min_temperature) return;
    ir_imaging_get_scale_color_index(buf_info->max_temperature, &color_index[0], &color_index[1]);
    ir_imaging_get_scale_color_index(buf_info->min_temperature, &color_index[2], &color_index[3]);
    if (color_index[0] == color_index[1]) {
        if (color_index[0] == 0) color_index[1] = 1;
        else color_index[0] = 2;
    }
    if (color_index[2] == color_index[3]) {
        if (color_index[2] == 0) color_index[3] = 1;
        else color_index[2] = 2;
    }
    if (color_index[0] == color_index[3]) {
        if (color_index[0] == 0) {
            color_index[1] = 1;
            color_index[3] = 1;
        } else {
            color_index[0] = 2;
            color_index[1] = 3;
        }
        grad_num = 1;
    } else if (color_index[0] == color_index[2]) grad_num = 1;
    else if (color_index[1] == color_index[2]) {
        color_index[2] = color_index[3];
        grad_num = 2;
    } else grad_num = 3;

    lv_draw_rect_dsc_init(&scale_dsc);
    scale_dsc.bg_opa = LV_OPA_COVER;
    scale_dsc.bg_grad.dir = LV_GRAD_DIR_VER;
    grad_item_height = IR_IMG_SCALE_HEIGHT / grad_num;
    for (i = 0; i < grad_num; i ++) {
        scale_dsc.bg_grad.stops[0].color = scale_color[color_index[i]];
        scale_dsc.bg_grad.stops[1].color = scale_color[color_index[i + 1]];
        lv_canvas_draw_rect(ir_img_canvas, IR_IMG_SCALE_X, IR_IMG_SCALE_Y + (i * grad_item_height), IR_IMG_SCALE_WIDTH, grad_item_height, &scale_dsc);
    }

    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = &puhui_55_10;
    label_dsc.align = LV_TEXT_ALIGN_CENTER;
    
    lv_snprintf(tmp_buf, 32, "%d ℃", scale_temperature[color_index[0]]);
    label_dsc.color = scale_color[color_index[0]];
    lv_canvas_draw_text(ir_img_canvas, IR_IMG_LABEL_X, IR_IMG_LABEL_Y1, IR_IMG_LABEL_WIDTH, &label_dsc, tmp_buf);

    lv_snprintf(tmp_buf, 32, "%d ℃", scale_temperature[color_index[3]]);
    label_dsc.color = scale_color[color_index[3]];
    lv_canvas_draw_text(ir_img_canvas, IR_IMG_LABEL_X, IR_IMG_LABEL_Y2, IR_IMG_LABEL_WIDTH, &label_dsc, tmp_buf);

    lv_label_set_text_fmt(max_temp_label, "Max:%d.%d℃", (int)buf_info->max_temperature, ((int)buf_info->max_temperature * 10) % 10);
    lv_obj_set_style_text_color(max_temp_label, ir_imaging_get_temperature_color(buf_info->max_temperature), LV_PART_MAIN);
    lv_label_set_text_fmt(min_temp_label, "Min:%d.%d℃", (int)buf_info->min_temperature, ((int)buf_info->min_temperature * 10) % 10);
    lv_obj_set_style_text_color(min_temp_label, ir_imaging_get_temperature_color(buf_info->min_temperature), LV_PART_MAIN);
}
/// @brief 绘制红外温度成像图片
/// @param t_buf 红外获取到的温度数据
/// @param layout_cfg 温度成像布局配置
static void ir_imaging_draw_img(ir_imaging_buf_layout_config_t *layout_cfg, ir_imaging_buf_info_t *buf_info)
{
    lv_coord_t i, j;
    lv_coord_t x, y;
    lv_draw_rect_dsc_t pixel_dsc;
    if (ir_img_canvas == NULL || layout_cfg == NULL || buf_info == NULL || buf_info->temperature_buf == NULL) return;

    lv_draw_rect_dsc_init(&pixel_dsc);
    x = IR_IMG_BORDER_SIZE + layout_cfg->x_offset;
    y = IR_IMG_BORDER_SIZE + layout_cfg->y_offset;
    pixel_dsc.bg_opa = LV_OPA_COVER;
    for (i = 0; i < layout_cfg->col_num; i++) {
        for (j = 0; j < layout_cfg->row_num; j++) {
            pixel_dsc.bg_color = ir_imaging_get_temperature_color(buf_info->temperature_buf[i * layout_cfg->row_num + j]);
            lv_canvas_draw_rect(ir_img_canvas, x + j * layout_cfg->pixel_size, y + i * layout_cfg->pixel_size, layout_cfg->pixel_size, layout_cfg->pixel_size, &pixel_dsc);
        }
    }
}
/// @brief 绘制目标图标
/// @param point 对应点
/// @param layout_cfg 温度成像布局配置
static void ir_imaging_draw_target(lv_point_t *point, ir_imaging_buf_layout_config_t *layout_cfg, ir_imaging_buf_info_t *buf_info)
{
    int index;
    lv_draw_img_dsc_t img_dsc;
    if (ir_img_canvas == NULL || point == NULL || layout_cfg == NULL || buf_info == NULL) return;

    index = ir_imaging_get_temperature_index(point, layout_cfg);
    if (index >= 0) {
        lv_draw_img_dsc_init(&img_dsc);
        img_dsc.opa = LV_OPA_50;
        lv_canvas_draw_img(ir_img_canvas, point->x - 12, point->y - 12, "A:/home/wicevi/lv_sim_vscode_sdl/apps/infrared_imaging/imgs/target.png", &img_dsc);
        lv_label_set_text_fmt(target_temp_label, "%d.%d℃", (int)buf_info->temperature_buf[index], ((int)buf_info->temperature_buf[index] * 10) % 10);
        lv_obj_set_style_text_color(target_temp_label, ir_imaging_get_temperature_color(buf_info->temperature_buf[index]), LV_PART_MAIN);
        target_point.x = point->x;
        target_point.y = point->y;
    }
}
/// @brief 画布点击事件
/// @param event 
static void ir_imaging_canvas_click_event_cb(lv_event_t *event)
{
    lv_point_t point;
    ir_imaging_buf_info_t *buf_info = (ir_imaging_buf_info_t *)lv_event_get_user_data(event);
    if (ir_img_canvas == NULL || buf_info == NULL) return;

    lv_tl_indev_get_absolute_point(&point, ir_img_canvas, lv_scr_act());
    ir_imaging_draw_target(&point, &buf_layout_config, buf_info);
}
/// @brief 红外成像应用相关数据刷新定时器回调函数
/// @param timer 定时器对象
static void ir_imaging_info_refresh_timer(lv_timer_t *timer)
{
    static uint8_t last_cnt_states = 1;
    ir_imaging_buf_info_t *buf_info = (ir_imaging_buf_info_t *)timer->user_data;
    if (buf_info == NULL || ir_imaging_global_layout == NULL) return;
    
    if (lv_obj_has_flag(ir_imaging_global_layout, LV_OBJ_FLAG_HIDDEN)) return;
    if (last_cnt_states != buf_info->cnt_states) {
        last_cnt_states = buf_info->cnt_states;
        if (buf_info->cnt_states) {
            //TODO: 显示正常状态操作按钮

        } else {
            //TODO: 显示重新连接按钮
            
        }
    }
    if (buf_info->cnt_states && buf_info->upate_flag) {
        ir_imaging_draw_scale(buf_info);
        ir_imaging_draw_img(&buf_layout_config, buf_info);
        ir_imaging_draw_target(&target_point, &buf_layout_config, buf_info);
        // ir_imaging_draw_target(&(lv_point_t){50, 50}, &buf_layout_config);
        buf_info->upate_flag = 0;
    }
}
/// @brief 红外成像销毁事件回调
/// @param event 事件对象
static void ir_imaging_self_destroy_event_cb(lv_event_t *event)
{
    ir_imaging_buf_info_t *buf_info = (ir_imaging_buf_info_t *)lv_event_get_user_data(event);
    if (buf_info_refresh_timer != NULL) {
        lv_timer_del(buf_info_refresh_timer);
        buf_info_refresh_timer = NULL;
    }
    if (buf_info && buf_info->disconnect_func) buf_info->disconnect_func();
    ir_imaging_global_layout = NULL;
}
/// @brief 收到菜单按钮事件
/// @param event 事件对象
static void ir_imaging_self_menu_event_cb(lv_event_t *event)
{
    const char *menu_str = lv_event_get_param(event);

    if (menu_str != NULL) {
        if (strncmp(menu_str, "BACK", 4) == 0) {
            //TODO: 弹窗确认是否退出

            lv_obj_del(ir_imaging_global_layout);
        }
    }
}
/// @brief 创建红外成像应用并返回全局容器对象
/// @param parent 父布局对象
/// @param user_data 应用的配置信息
/// @return 应用的全局容器对象
lv_obj_t *ir_imaging_create_main_page(lv_obj_t *parent, void *user_data)
{
    lv_obj_t *tmp_layout = NULL, *ctrl_btn = NULL;
    lv_coord_t parent_w = LV_SIZE_CONTENT, parent_h = LV_SIZE_CONTENT;
    ir_imaging_config_t *ir_imaging_config = NULL;

    if (user_data == NULL || ir_imaging_global_layout != NULL) return NULL;
    if (parent == NULL) parent = lv_scr_act();

    ir_imaging_config = (ir_imaging_config_t *)user_data;
    //TODO: 检查配置信息

    parent_w = lv_obj_get_width(parent);
    parent_h = lv_obj_get_height(parent);

    ir_imaging_global_layout = lv_tl_create_base_layout(parent, parent_w, parent_h);
    lv_obj_center(ir_imaging_global_layout);
    lv_obj_set_style_bg_color(ir_imaging_global_layout, global_bg_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ir_imaging_global_layout, LV_OPA_COVER, LV_PART_MAIN);

    ir_img_canvas = lv_canvas_create(ir_imaging_global_layout);
    lv_obj_align(ir_img_canvas, LV_ALIGN_TOP_MID, 0, 0);
    lv_canvas_set_buffer(ir_img_canvas, ir_img_buf, IR_IMG_CANVAS_WIDTH, IR_IMG_CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_canvas_fill_bg(ir_img_canvas, canvas_bg_color, LV_OPA_COVER);
    lv_obj_set_style_shadow_color(ir_img_canvas, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(ir_img_canvas, 12, LV_PART_MAIN);
    lv_obj_add_flag(ir_img_canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ir_img_canvas, ir_imaging_canvas_click_event_cb, LV_EVENT_CLICKED, ir_imaging_config->ir_img_buf_info);

    tmp_layout = lv_tl_create_base_layout(ir_imaging_global_layout, parent_w, 16);
    lv_obj_align(tmp_layout, LV_ALIGN_TOP_MID, 0, IR_IMG_CANVAS_HEIGHT + 6);
    lv_obj_set_flex_flow(tmp_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tmp_layout, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(tmp_layout, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_right(tmp_layout, 4, LV_PART_MAIN);
    
    min_temp_label = lv_label_create(tmp_layout);
    lv_obj_set_style_text_font(min_temp_label, &puhui_55_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(min_temp_label, ir_imaging_get_temperature_color(-40), LV_PART_MAIN);
    lv_label_set_text(min_temp_label, "Min:-40℃");

    target_temp_label = lv_label_create(tmp_layout);
    lv_obj_set_style_text_font(target_temp_label, &puhui_55_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(target_temp_label, ir_imaging_get_temperature_color(0), LV_PART_MAIN);
    lv_label_set_text(target_temp_label, "---.-℃");

    max_temp_label = lv_label_create(tmp_layout);
    lv_obj_set_style_text_font(max_temp_label, &puhui_55_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(max_temp_label, ir_imaging_get_temperature_color(300), LV_PART_MAIN);
    lv_label_set_text(max_temp_label, "Max:300℃");

    tmp_layout = lv_tl_create_base_layout(ir_imaging_global_layout, parent_w, LV_SIZE_CONTENT);
    lv_obj_align(tmp_layout, LV_ALIGN_TOP_MID, 0, IR_IMG_CANVAS_HEIGHT + 12 + 16);

    // discnt_show_layout = lv_tl_create_base_layout(tmp_layout, parent_w, LV_SIZE_CONTENT);
    // lv_obj_align(discnt_show_layout, LV_ALIGN_TOP_MID, 0, 0);
    // lv_obj_set_flex_flow(discnt_show_layout, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(discnt_show_layout, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // lv_obj_t *discnt_tip_label = lv_label_create(discnt_show_layout);
    // lv_obj_set_style_text_font(max_temp_label, &puhui_55_12, LV_PART_MAIN);
    // lv_obj_set_style_pad_all(discnt_tip_label, 8, LV_PART_MAIN);
    // lv_obj_set_style_text_color(discnt_tip_label, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    // lv_label_set_text(discnt_tip_label, "Device disconnected!");

    // lv_obj_t *cnt_btn = lv_btn_create(discnt_show_layout);
    // lv_obj_t *cnt_btn_label = lv_label_create(cnt_btn);
    // lv_label_set_text(cnt_btn_label, "Try Connect");

    ctrl_btn_layout = lv_tl_create_base_layout(tmp_layout, parent_w, LV_SIZE_CONTENT);
    lv_obj_align(ctrl_btn_layout, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(ctrl_btn_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_btn_layout, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ctrl_btn = lv_btn_create(ctrl_btn_layout);
    lv_obj_t *ctrl_btn_label = lv_label_create(ctrl_btn);
    lv_label_set_text(ctrl_btn_label, "Save");

    ctrl_btn = lv_btn_create(ctrl_btn_layout);
    ctrl_btn_label = lv_label_create(ctrl_btn);
    lv_label_set_text(ctrl_btn_label, "Record");

    ctrl_btn = lv_btn_create(ctrl_btn_layout);
    ctrl_btn_label = lv_label_create(ctrl_btn);
    lv_label_set_text(ctrl_btn_label, "Clean");

    buf_layout_config.row_num = ir_imaging_config->row_num;
    buf_layout_config.col_num = ir_imaging_config->col_num;
    buf_layout_config.pixel_size = LV_MIN((IR_IMG_BUF_WIDTH / buf_layout_config.row_num), IR_IMG_BUF_HEIGHT / buf_layout_config.col_num);
    buf_layout_config.x_offset = (IR_IMG_BUF_WIDTH - (buf_layout_config.pixel_size * buf_layout_config.row_num)) / 2;
    buf_layout_config.y_offset = (IR_IMG_BUF_HEIGHT - (buf_layout_config.pixel_size * buf_layout_config.col_num)) / 2;

    buf_info_refresh_timer = lv_timer_create(ir_imaging_info_refresh_timer, IR_IMG_REFRESH_TIME, ir_imaging_config->ir_img_buf_info);
    lv_obj_add_event_cb(ir_imaging_global_layout, ir_imaging_self_destroy_event_cb, LV_EVENT_DELETE, ir_imaging_config->ir_img_buf_info);
    lv_obj_add_event_cb(ir_imaging_global_layout, ir_imaging_self_menu_event_cb, LV_EVENT_CANCEL, NULL);
    return ir_imaging_global_layout;
}


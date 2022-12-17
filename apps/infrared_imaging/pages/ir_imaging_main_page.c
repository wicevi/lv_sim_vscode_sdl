#include <math.h>
#include "lvgl_tool.h"
#include "../infrared_imaging.h"

#define IR_IMG_SCALE_300_COLOR  LV_COLOR_MAKE(0xff, 0x00, 0x00)
#define IR_IMG_SCALE_50_COLOR   LV_COLOR_MAKE(0xf7, 0x94, 0x1e)
#define IR_IMG_SCALE_20_COLOR   LV_COLOR_MAKE(0x00, 0x2a, 0x88)
#define IR_IMG_SCALE_N40_COLOR  LV_COLOR_MAKE(0x00, 0xcd, 0xe9)
#define IR_IMG_GLOBAL_BG_COLOR  LV_COLOR_MAKE(0xe6, 0xe6, 0xe6)
#define IR_IMG_CANVAS_BG_COLOR  LV_COLOR_MAKE(0xF6, 0xF6, 0xF6)

#define IR_IMG_BORDER_SIZE      (5)
#define IR_IMG_CANVAS_WIDTH     (240)
#define IR_IMG_CANVAS_HEIGHT    (144 + IR_IMG_BORDER_SIZE * 2)
#define IR_IMG_BUF_WIDTH        (192)
#define IR_IMG_BUF_HEIGHT       (144)
#define IR_IMG_LABEL_WIDTH      (36)
#define IR_IMG_LABEL_HEIGHT     (14)
#define IR_IMG_LABEL_X          (IR_IMG_CANVAS_WIDTH - IR_IMG_LABEL_WIDTH - (IR_IMG_CANVAS_WIDTH - IR_IMG_BUF_WIDTH - IR_IMG_BORDER_SIZE - IR_IMG_LABEL_WIDTH) / 2)
#define IR_IMG_LABEL_Y1         (IR_IMG_SCALE_Y - 16)
#define IR_IMG_LABEL_Y2         (IR_IMG_SCALE_Y + IR_IMG_SCALE_HEIGHT - 3)
#define IR_IMG_SCALE_X          (IR_IMG_LABEL_X + 6)
#define IR_IMG_SCALE_Y          (IR_IMG_BORDER_SIZE + IR_IMG_LABEL_HEIGHT)
#define IR_IMG_SCALE_WIDTH      (24)
#define IR_IMG_SCALE_HEIGHT     (IR_IMG_BUF_HEIGHT - IR_IMG_LABEL_HEIGHT * 2)

LV_FONT_DECLARE(puhui_55_10);

static lv_obj_t *ir_imaging_global_layout = NULL;
static lv_obj_t *ir_img_canvas = NULL;
static lv_color_t ir_img_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(IR_IMG_CANVAS_WIDTH, IR_IMG_CANVAS_HEIGHT)];
const lv_color_t global_bg_color = IR_IMG_GLOBAL_BG_COLOR;
const lv_color_t canvas_bg_color = IR_IMG_CANVAS_BG_COLOR;
const lv_color_t scale_color[] = { IR_IMG_SCALE_300_COLOR, IR_IMG_SCALE_50_COLOR, IR_IMG_SCALE_20_COLOR, IR_IMG_SCALE_N40_COLOR };
const int32_t scale_temperature[] = { 300, 50, 20, -40 };

static void ir_imaging_get_scale_color_index(float temperature, uint8_t *i1, uint8_t *i2)
{
    if (temperature > 300) {
        *i1 = 0;
        *i2 = 0;
    } else if (temperature > 50) {
        *i1 = 0;
        *i2 = 1;
    } else if (temperature > 20) {
        *i1 = 1;
        *i2 = 2;
    } else if (temperature > -40) {
        *i1 = 2;
        *i2 = 3;
    } else {
        *i1 = 3;
        *i2 = 3;
    }
}

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

static void ir_imaging_draw_scale(float max_temperature, float min_temperature)
{
    char tmp_buf[32] = {0};
    uint8_t color_index[4] = {0};
    uint8_t grad_num = 0, i = 0;
    lv_coord_t grad_item_height = 0;
    lv_draw_rect_dsc_t scale_dsc;
    lv_draw_label_dsc_t label_dsc;

    if (ir_img_canvas == NULL) return;
    ir_imaging_get_scale_color_index(max_temperature, &color_index[0], &color_index[1]);
    ir_imaging_get_scale_color_index(min_temperature, &color_index[2], &color_index[3]);
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
}

static void ir_imaging_draw_img(ir_imaging_config_t *ir_img_cfg)
{
    lv_coord_t i, j;
    lv_coord_t x, y;
    lv_draw_rect_dsc_t pixel_dsc;
    if (ir_img_canvas == NULL || ir_img_cfg == NULL) return;

    lv_draw_rect_dsc_init(&pixel_dsc);
    x = IR_IMG_BORDER_SIZE + ir_img_cfg->x_offset;
    y = IR_IMG_BORDER_SIZE + ir_img_cfg->y_offset;
    pixel_dsc.bg_opa = LV_OPA_COVER;
    for (i = 0; i < ir_img_cfg->col_num; i++) {
        for (j = 0; j < ir_img_cfg->row_num; j++) {
            pixel_dsc.bg_color = ir_imaging_get_temperature_color(ir_img_cfg->temperature_buf[i * ir_img_cfg->row_num + j]);
            lv_canvas_draw_rect(ir_img_canvas, x + j * ir_img_cfg->pixel_size, y + i * ir_img_cfg->pixel_size, ir_img_cfg->pixel_size, ir_img_cfg->pixel_size, &pixel_dsc);
        }
    }
}

lv_obj_t *ir_imaging_create_main_page(lv_obj_t *parent, void *user_data)
{
    lv_coord_t parent_w = LV_SIZE_CONTENT, parent_h = LV_SIZE_CONTENT;
    ir_imaging_config_t *ir_imaging_config = NULL;

    LV_LOG_USER("parent: %p, user_data: %p.", parent, user_data);
    if (user_data == NULL || ir_imaging_global_layout != NULL) return NULL;
    if (parent == NULL) parent = lv_scr_act();

    ir_imaging_config = (ir_imaging_config_t *)user_data;
    if (ir_imaging_config->temperature_buf == NULL || ir_imaging_config->row_num == 0 || ir_imaging_config->col_num == 0) return NULL;
    if (ir_imaging_config->max_temperature < ir_imaging_config->min_temperature) return NULL;
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

    ir_imaging_config->pixel_size = LV_MIN((IR_IMG_BUF_WIDTH / ir_imaging_config->row_num), IR_IMG_BUF_HEIGHT / ir_imaging_config->col_num);
    ir_imaging_config->x_offset = (IR_IMG_BUF_WIDTH - (ir_imaging_config->pixel_size * ir_imaging_config->row_num)) / 2;
    ir_imaging_config->y_offset = (IR_IMG_BUF_HEIGHT - (ir_imaging_config->pixel_size * ir_imaging_config->col_num)) / 2;

    //Test data create
    // float diff = (ir_imaging_config->max_temperature - ir_imaging_config->min_temperature) / (float)(ir_imaging_config->row_num * ir_imaging_config->col_num);
    // for (int i = 0; i < ir_imaging_config->row_num * ir_imaging_config->col_num; i++) {
    //     ir_imaging_config->temperature_buf[i] = ir_imaging_config->min_temperature + (float)i * diff;
    // }

    ir_imaging_draw_scale(ir_imaging_config->max_temperature, ir_imaging_config->min_temperature);
    ir_imaging_draw_img(ir_imaging_config);
    
    return ir_imaging_global_layout;
}


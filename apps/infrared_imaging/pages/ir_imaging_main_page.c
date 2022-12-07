#include "../infrared_imaging.h"

#define IR_IMG_CANVAS_WIDTH     224
#define IR_IMG_CANVAS_HEIGHT    168

static lv_color_t ir_img_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(IR_IMG_CANVAS_WIDTH, IR_IMG_CANVAS_HEIGHT)];

void ir_imaging_create_main_page(lv_obj_t *parent)
{
    if (parent == NULL) return;
    lv_obj_clean(parent);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = 10;
    rect_dsc.bg_opa = LV_OPA_10;
    rect_dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
    rect_dsc.bg_grad.stops[0].color = lv_palette_main(LV_PALETTE_RED);
    rect_dsc.bg_grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
    rect_dsc.border_width = 2;
    rect_dsc.border_opa = LV_OPA_10;
    rect_dsc.border_color = lv_color_white();
    rect_dsc.shadow_width = 5;
    rect_dsc.shadow_ofs_x = 5;
    rect_dsc.shadow_ofs_y = 5;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_palette_main(LV_PALETTE_ORANGE);

    lv_obj_t * canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, ir_img_buf, IR_IMG_CANVAS_WIDTH, IR_IMG_CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
    // lv_canvas_set_palette(canvas, 0, lv_color_hex(0x00f230));
    // lv_canvas_set_palette(canvas, 1, lv_palette_main(LV_PALETTE_BLUE));
    
    /*Red background (There is no dedicated alpha channel in indexed images so LV_OPA_COVER is ignored)*/
    lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_COVER);

    lv_canvas_draw_rect(canvas, 70, 60, 100, 70, &rect_dsc);
    lv_canvas_draw_text(canvas, 50, 30, 100, &label_dsc, "Some text on text canvas");
}


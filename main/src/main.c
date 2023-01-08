
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lv_examples/lv_demo.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"

static void hal_init(void);
static int tick_thread(void *data);

#include "apps/infrared_imaging/infrared_imaging.h"
#include "apps/desktop/desktop.h"
#include <math.h>

float temp_buf[1024] = {0};

static desktop_app_config_t setting_app_config = {
    .name = "setting",
    .icon_dsc = NULL,
    .icon_path = "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/setting.png",
    .icon_zoom = 172,
    .position = 7
};

static ir_imaging_buf_info_t ir_img_buf_info = {
    .temperature_buf = temp_buf,
    .max_temperature = 248.9,
    .min_temperature = -20.6,
    .upate_flag = 1,
    .cnt_states = 1,
    .connect_func = NULL,
    .disconnect_func = NULL
};
static ir_imaging_config_t ir_image_config = {
    .row_num = 32,
    .col_num = 24,
    .ir_img_buf_info = &ir_img_buf_info
};
static desktop_app_config_t ir_image_app_config = {
    .name = "IR image",
    .icon_dsc = NULL,
    .icon_path = "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/ir_image.png",
    .icon_zoom = 180,
    .position = 4,
    .create_func = ir_imaging_create_main_page,
    .user_data = &ir_image_config
};
static desktop_app_config_t ota_app_config = {
    .name = "Shell",
    .icon_dsc = NULL,
    .icon_path = "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/shell.png",
    .icon_zoom = 180,
    .position = 10
};

static desktop_app_config_t *desktop_app_configs[] = {
    &setting_app_config, 
    &ir_image_app_config, 
    &ota_app_config
};
static desktop_top_bar_info_t top_bar_info = {0};
static desktop_create_config_t desktop_config = {
    .bg_img_dsc = NULL,
    .bg_img_path = "A:/home/wicevi/lv_sim_vscode_sdl/apps/desktop/imgs/desktop_bg1.png",
    .item_size = 64,
    .item_name_color = LV_COLOR_MAKE(0XFF, 0XFF, 0XFF),
    .app_num = 3,
    .row_num = 3,
    .is_auto_y_offset = 1,
    .y_space_offset = 0,
    .app_configs = desktop_app_configs,
    .top_bar_info = &top_bar_info
};

int main(int argc, char **argv)
{
    uint32_t time_ms = 0;
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();
    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    // ir_imaging_create_main_page(lv_scr_act());
    desktop_create_main_page(NULL, &desktop_config);
    while(1) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
        temp_buf[rand() % 768] += 1.0f;
        time_ms += 5;
        if ((time_ms % 100) < 5 && top_bar_info.bat_percentage < 100) top_bar_info.bat_percentage++;
        if ((time_ms % 1000) < 5) {
            top_bar_info.time_s++;
            ir_img_buf_info.upate_flag = 1;
        } if (time_ms >= 10000) {
            top_bar_info.usb_is_cnt = rand() % 2;
            top_bar_info.wifi_is_cnt = rand() % 2;
            top_bar_info.ble_is_cnt = rand() % 2;
            top_bar_info.is_warning = rand() % 2;
            top_bar_info.is_charging = rand() % 2;
            time_ms = 0;
            ir_img_buf_info.cnt_states = !ir_img_buf_info.cnt_states;
        }
    }

    return 0;
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void)
{
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();
    /* Tick init.
    * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
    * how much time were elapsed Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /*Create a display buffer*/
    static lv_disp_draw_buf_t disp_buf1;
    static lv_color_t buf1_1[MONITOR_HOR_RES * 100];
    static lv_color_t buf1_2[MONITOR_HOR_RES * 100];
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, buf1_2, MONITOR_HOR_RES * 100);

    /*Create a display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = monitor_flush;
    disp_drv.hor_res = MONITOR_HOR_RES;
    disp_drv.ver_res = MONITOR_VER_RES;
    disp_drv.antialiasing = 1;

    lv_disp_t * disp = lv_disp_drv_register(&disp_drv);

    lv_theme_t * th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    keyboard_init();
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);
    mousewheel_init();
    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = mousewheel_read;

    lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);

    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
    (void)data;

    while(1) {
        SDL_Delay(5);
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

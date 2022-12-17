#ifndef DESKTOP_H
#define DESKTOP_H

#include "lvgl.h"

/// @brief 要在桌面上显示的应用配置结构体
typedef struct
{
    const char *name;               //应用名称
    const lv_img_dsc_t *icon_dsc;   //应用图标对象  TODO: 对象和路径可以整合为一个void *指针
    const char *icon_path;          //应用图标路径
    uint16_t icon_zoom;             //应用缩放比例（255为1）
    uint8_t position;               //应用位置(冲突后者不显示)
    lv_obj_t *(*create_func)(lv_obj_t *parent, void *user_data);    //应用创建（启动）函数
    void *user_data;                //应用启动函数参数
} desktop_app_config_t;
/// @brief 桌面应用的配置信息
typedef struct
{
    const lv_img_dsc_t *bg_img_dsc; //桌面背景图片对象  TODO: 多背景图片轮换功能
    const char *bg_img_path;        //桌面背景图片路径  TODO: 对象和路径可以整合为一个void *指针
    lv_coord_t item_size;           //桌面显示的APP应用大小
    lv_color_t item_name_color;     //桌面显示的APP应用名称颜色
    uint8_t app_num;                //桌面显示的APP应用数量
    uint8_t row_num;                //桌面单行显示的APP应用数量
    uint8_t is_auto_y_offset;       //是否自动补偿Y轴间隔大小
    lv_coord_t y_space_offset;      //指定Y轴间隔大小的偏移量（若启用is_auto_y_offset，该项无效）
    desktop_app_config_t **app_configs;
} desktop_create_config_t;
/// @brief 桌面的APP应用布局配置
typedef struct
{
    uint8_t row_num;                //桌面实际单行显示的APP应用数量
    uint8_t position_num;           //桌面位置总数量
    bool *position_state;           //桌面各个位置状态（true：已存在APP false：空）
    lv_coord_t item_size;           //桌面实际显示的APP应用大小
    lv_coord_t x_space_size;        //两个应用之间X轴间隔
    lv_coord_t y_space_size;        //两个应用之间Y轴间隔
    lv_coord_t height_offset;       //应用移动空间高度偏移
} desktop_app_layout_config_t;
/// @brief 桌面的APP应用缓存信息结构体
typedef struct
{
    lv_obj_t *app_content_layout;   //APP全局容器对象
    void *create_func_prt;          //APP创建（启动）函数指针
} desktop_app_cache_t;

lv_obj_t *desktop_create_main_page(lv_obj_t *parent, void *user_data);

#endif
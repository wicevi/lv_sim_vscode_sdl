#ifndef IR_IMAGING_H
#define IR_IMAGING_H

#include "lvgl.h"

/// @brief 获取到的红外温度及设备状态
typedef struct
{
    float *temperature_buf;             //温度数据
    float max_temperature;              //最大温度
    float min_temperature;              //最小温度
    uint8_t upate_flag;                 //是否已更新（0：未更新 1：已更新）
    uint8_t cnt_states;                 //连接状态（0：断开 1：连接）
    uint8_t (*connect_func)(void);      //连接设备函数
    uint8_t (*disconnect_func)(void);   //断开设备函数
} ir_imaging_buf_info_t;
/// @brief 红外成像应创建的配置信息
typedef struct
{
    lv_coord_t row_num, col_num;                //温度矩阵的行列数量
    ir_imaging_buf_info_t *ir_img_buf_info;     //相关信息
} ir_imaging_config_t;
/// @brief 温度成像布局配置
typedef struct
{
    lv_coord_t row_num, col_num;        //温度矩阵的行列数量
    lv_coord_t pixel_size;              //每个温度像素大小
    lv_coord_t x_offset;                //X轴偏移
    lv_coord_t y_offset;                //Y轴偏移
} ir_imaging_buf_layout_config_t;

lv_obj_t *ir_imaging_create_main_page(lv_obj_t *parent, void *user_data);

#endif

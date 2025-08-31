#pragma once
#include "board.h"
#include "lvgl.h"

#if (LV_USE_OS != LV_OS_FREERTOS)
#error "LV_USE_OS must be set, or lv_lock() has no effect"
#endif

#define WITH_LV_LOCK(code_block) \
	do { \
        lv_lock(); \
        code_block; \
        lv_unlock(); \
    } while (0)


// 创建两个藏在屏幕角落（圆形屏幕未显示区域）的 UI 按钮来接收按钮事件
void lv_port_init_default_button();
void lv_port_init_indev();
void lv_port_init_display(esp_lcd_panel_handle_t panel, esp_lcd_panel_io_handle_t io_panel);
void lv_port_run_task();

static inline void lv_port() {
	lv_init();
	lv_port_init_display(lcd_get_panel_handle(), lcd_get_panel_io_handle());
	lv_port_init_indev();
	lv_port_init_default_button();
	lv_port_run_task();
}

void button_indev_set_left_button_point(int x, int y);
void button_indev_set_right_button_point(int x, int y);
void button_indev_set_default_button_points();

lv_indev_t* get_button_indev();
lv_indev_t* get_encoder_indev();
lv_obj_t* get_default_left_button();
lv_obj_t* get_default_right_button();

int encoder_indev_get_step();
void encoder_indev_set_step(int s);
void encoder_indev_set_group(lv_group_t* g);
typedef void (*encoder_indev_cb_t)(int i);
void encoder_indev_set_cb(encoder_indev_cb_t cb);

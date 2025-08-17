#pragma once
#include "lvgl.h"
#include "lv_app_multi_lang.h"

/*
	每个 app 可以有一个或多个屏幕（screen).
	screen_data 用来存储每个屏幕生命周期中使用的临时变量,
	在屏幕打开时由系统分配给 screen_data_size 大小的空内存，用户须自行在 open 函数中初始化之，
	屏幕关闭时调用 close 函数，之后 screen_data 会由系统释放。
*/
typedef void (*lv_screen_open_cb_t)(lv_obj_t* scr, void* screen_data);
typedef void (*lv_screen_cb_t)(void* screen_data);
typedef struct {
	lv_screen_open_cb_t open;
	lv_screen_cb_t close;
	size_t screen_data_size;
} lv_screen_t;


/*
	 app 的名字、图标等信息，
*/
typedef struct {
	char* name[LV_APP_LANG_MAX];
	char* icon_font;
	void* icon_img;
	lv_screen_t* screen; // 指定一个初始屏幕
	void* user_data;

	// screen_data 用户不应直接操作
	void* screen_data;
} lv_app_t;


/*
	 app 切换的回调函数
*/
typedef void (*lv_app_changed_cb_t) (lv_app_t* app);
extern lv_app_changed_cb_t lv_app_changed_cb;


/*
	app 操作
*/
lv_app_t* lv_app_get_current();
void lv_app_open(lv_app_t* app);
void lv_app_open_prev();
void lv_app_refresh();
void lv_app_set_screen(lv_screen_t* screen);

// 获取 当前 app 的 screen_data
static inline void* lv_app_get_screen_data() {
	return lv_app_get_current()->screen_data;
}

static inline void lv_app_set_user_data(lv_app_t* app, void* user_data) {
	if(app) app->user_data = user_data;
}
static inline void* lv_app_get_user_data(lv_app_t* app) {
	if(app) return app->user_data;
	return NULL;
}

void lv_app_init();
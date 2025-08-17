#pragma once 

/* 
	多语言设置 
*/
typedef enum {
	LV_APP_LANG_EN,
	LV_APP_LANG_ZH,

	LV_APP_LANG_MAX,
} lv_app_lang_t;


int lv_app_get_lang();
void lv_app_set_lang(int lang);

#define LV_APP_LANG(a, b) ({static const char* const _lang_arr[]={a, b}; _lang_arr[lv_app_get_lang()];})


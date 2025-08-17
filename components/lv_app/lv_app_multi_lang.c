#include "lv_app.h"
#include "esp_log.h"

static const char* const TAG ="lv_app_lang";

static int _lv_app_lang_id;
int lv_app_get_lang() {return _lv_app_lang_id;}
void lv_app_set_lang(int lang) {
    if(_lv_app_lang_id == lang) return;
    if(0>lang || lang>=LV_APP_LANG_MAX) {
        ESP_LOGE(TAG, "setting language id [%d] out of range.");
        return;
    }
    _lv_app_lang_id = lang;
    lv_app_refresh();
}
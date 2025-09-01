#include "apps.h"

typedef struct {

} screen_data_t;


static void open_screen(lv_obj_t* scr, screen_data_t* data);
static void close_screen(screen_data_t* data);
static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};


lv_app_t ai_chat_app = {
    .name = {"AI chat", "你好小智"},
    .icon_img = &smiley_icon_40x40,
    .icon_font = "🤖",
    .screen = &screen,
};


static void button_event_handler(lv_event_t * e) {
    lv_obj_t* target = lv_event_get_target(e);
    screen_data_t* data = lv_event_get_user_data(e);

    if(target == get_default_right_button()) {

    } else if(target == get_default_left_button()) {
        lv_app_open(&home_app);
    }
}


/*
    屏幕打开时由系统分配给屏幕对象 scr 和内存空间 data,
    开发者自行在屏幕里添加 UI 元素，初始化临时变量 data
*/
static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    lv_obj_t* o = lv_label_create(scr);
    lv_obj_center(o);
    lv_label_set_text_static(o, "to be done ...");
    lv_obj_add_event_cb(get_default_left_button(), button_event_handler, LV_EVENT_CLICKED, data);
    lv_obj_add_event_cb(get_default_right_button(), button_event_handler, LV_EVENT_CLICKED, data);
}


/*
    屏幕关闭后屏幕对象及其子元素会由系统销毁，并释放屏幕的临时变量 data.
    如果有其他需要手动释放的资源，或需要保存的永久变量，在 close 回调函数里完成
*/
static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_right_button(), LV_EVENT_ALL);
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
}
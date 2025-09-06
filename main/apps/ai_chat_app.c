/*
    这个例子演示小智ai对话应用
    参考 https://github.com/hyansuper/xiaozhi_chat/tree/main/example
*/

#include "apps.h"

typedef struct {
    lv_obj_t* lbl_emoji;
    lv_obj_t* lbl_state;
} screen_data_t;

static void button_event_handler(lv_event_t * e) {
    lv_obj_t* target = lv_event_get_target(e);
    screen_data_t* data = lv_event_get_user_data(e);

    if(target == get_default_right_button()) {
        // xz_chat_toggle_chat_state(chat);
        
    } else if(target == get_default_left_button()) {
        lv_app_open(&home_app);
    }
}


static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    data->lbl_emoji = lv_label_create(scr);
    lv_obj_center(data->lbl_emoji);
    lv_label_set_text_static(data->lbl_emoji, "to do ...");
    // data->lbl_state = lv_label_create(scr);
    // lv_obj_align(data->lbl_state, LV_ALIGN_CENTER, 0, -60);
    // lv_label_set_text_static(data->lbl_state, "idle");

    lv_obj_add_event_cb(get_default_left_button(), button_event_handler, LV_EVENT_LONG_PRESSED, data);
    lv_obj_add_event_cb(get_default_right_button(), button_event_handler, LV_EVENT_CLICKED, data);
}

static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_right_button(), LV_EVENT_ALL);
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
    // xz_chat_exit_session(chat);
}

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

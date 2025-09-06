/*
    这个例子演示小智ai对话应用
    参考 https://github.com/hyansuper/xiaozhi_chat/tree/main/example
*/

#include "apps.h"
#include "ai_chat.h"
#include "ext_mjson.h"

typedef struct {
    lv_obj_t* lbl_info;
} screen_data_t;

static void button_event_handler(lv_event_t * e) {
    lv_obj_t* target = lv_event_get_target(e);
    screen_data_t* data = lv_event_get_user_data(e);

    if(target == get_default_right_button()) {
        xz_chat_toggle_chat_state(chat);
        
    } else if(target == get_default_left_button()) {
        lv_app_open(&home_app);
    }
}


static void xz_chat_on_event(xz_chat_event_t event, xz_chat_event_data_t *event_data, xz_chat_t* chat) {
    if(event==XZ_EVENT_ACTIVATION_CHECK_RESULT) {
        if(event_data->activation_check_err) {
            int activation_check_delay = (int)xz_chat_get_user_data(chat);
            activation_check_delay *= 2;
            if(activation_check_delay> 30000) activation_check_delay=30000;
            xz_chat_set_user_data(chat, (void*)activation_check_delay);
            vTaskDelay(pdMS_TO_TICKS(activation_check_delay));
            xz_chat_activation_check(chat, NULL);
        } else {
            free(ai_chat_activation_msg);
            ai_chat_activation_msg = NULL;
            xz_chat_start(chat);
        }
    } else if(event==XZ_EVENT_JSON_RECEIVED) {
        if(QESTREQL(event_data->type, "llm")) {
            char* emotion;
            int emotion_len;
            if((emjson_locate_string(event_data->json, event_data->len, "$.emotion", &emotion, &emotion_len))) {
                WITH_LV_LOCK({ lv_label_set_text_fmt(((screen_data_t*)lv_app_get_screen_data())->lbl_info, "[%.*s]", emotion_len, emotion); });
            }
        }
    }
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    xz_chat_set_event_cb(chat, xz_chat_on_event);
    
    data->lbl_info = lv_label_create(scr);
    lv_obj_center(data->lbl_info);
    if(ai_chat_activation_msg) {
        lv_label_set_text_static(data->lbl_info, ai_chat_activation_msg);
        xz_chat_set_user_data(chat, (void*)1000);   
        xz_chat_activation_check(chat, NULL);
    } else {
        lv_label_set_text_static(data->lbl_info, "[happy]");
    }

    lv_obj_add_event_cb(get_default_left_button(), button_event_handler, LV_EVENT_LONG_PRESSED, data);
    lv_obj_add_event_cb(get_default_right_button(), button_event_handler, LV_EVENT_CLICKED, data);
}

static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_right_button(), LV_EVENT_ALL);
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
    xz_chat_set_event_cb(chat, NULL);
    xz_chat_exit_session(chat);
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

/*
    这个例子演示小智ai对话应用
    参考 https://github.com/hyansuper/xiaozhi_chat/tree/main/example
*/

#include "apps.h"
#include "ai_chat.h"
#include "ext_mjson.h"

typedef struct {
    lv_obj_t* lbl_emotion;
    lv_obj_t* lbl_state;
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
        // see https://github.com/78/xiaozhi-esp32/blob/main/docs/websocket.md

        screen_data_t* screen_data = lv_app_get_screen_data();
        if(QESTREQL(event_data->type, "llm")) {
            char* emotion;
            int emotion_len;
            if((emjson_locate_string(event_data->json, event_data->len, "$.emotion", &emotion, &emotion_len))) {
                WITH_LV_LOCK({ lv_label_set_text_fmt(screen_data->lbl_emotion, "[%.*s]", emotion_len, emotion); });
            }
        } else if(QESTREQL(event_data->type, "tts")) {
            char* tts_state = emjson_find_string(event_data->json, event_data->len, "$.state");
            if(tts_state) {
                if(QESTREQL(tts_state, "start")) {
                    WITH_LV_LOCK({ lv_label_set_text_static(screen_data->lbl_state, "Speaking..."); });
                } else if(QESTREQL(tts_state, "stop")) {
                    WITH_LV_LOCK({ lv_label_set_text_static(screen_data->lbl_state, "Listening..."); });
                }
            }
        } else if(QESTREQL(event_data->type, "goodbye")) {
            WITH_LV_LOCK({ lv_label_set_text_static(screen_data->lbl_state, "Idle..."); });
        } else if(QESTREQL(event_data->type, "hello")) {
            WITH_LV_LOCK({ lv_label_set_text_static(screen_data->lbl_state, "Listening..."); });
        }
    }
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    char* chat_state = "";
    data->lbl_state = lv_label_create(scr);
    lv_obj_align(data->lbl_state, LV_ALIGN_CENTER, 0, -50);
    if(!xz_chat_is_in_session(chat))
        chat_state="Idle...";
    else if(xz_chat_is_listening(chat)) 
        chat_state = "Listening...";
    else if(xz_chat_is_speaking(chat))
        chat_state = "Speaking...";
    lv_label_set_text_static(data->lbl_state, chat_state);

    data->lbl_emotion = lv_label_create(scr);
    lv_obj_center(data->lbl_emotion);
    if(ai_chat_activation_msg) {
        lv_label_set_text_static(data->lbl_emotion, ai_chat_activation_msg);
        xz_chat_set_user_data(chat, (void*)1000);   
        xz_chat_activation_check(chat, NULL);
    } else {
        lv_label_set_text_static(data->lbl_emotion, "[neutral]");
    }

    xz_chat_set_event_cb(chat, xz_chat_on_event);
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

#include "apps.h"
#include "wifi_manager.h"

typedef struct {
    lv_obj_t* label;
    lv_obj_t* qrcode;
} screen_data_t;


static void ip_changed(lv_observer_t* obs, lv_subject_t* sub) {
    screen_data_t* data = lv_observer_get_user_data(obs);
    lv_label_set_text_fmt(data->label, "http://%s", wifi_mgr_ipstr());
    char* url = lv_label_get_text(data->label);
    lv_qrcode_update(data->qrcode, url, strlen(url));
    lv_obj_align_to(data->label, data->qrcode, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    lv_obj_t* qrcode = data->qrcode = lv_qrcode_create(scr);
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 0, -20);
    lv_qrcode_set_size(qrcode, 80);
    lv_obj_set_style_pad_all(qrcode, 4, 0);
    lv_obj_set_style_radius(qrcode, 6, 0);
    
    data->label = lv_label_create(scr);

    lv_subject_add_observer_obj(&wifi_sub, ip_changed, scr, data);
    lv_obj_add_event_cb(get_default_left_button(), back_to_home_app, LV_EVENT_CLICKED, NULL);
}

static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
}

static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};



lv_app_t settings_app = {
    .name = {"Settings", "设置"},
    .icon_img = &tools_icon_40x40,
    .icon_font = "⚙️",
    .screen = &screen,
};

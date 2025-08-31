#include "apps.h"

typedef struct {
    lv_obj_t* lbl_time;
    lv_obj_t* hr_hand;
    lv_obj_t* min_hand;
    lv_obj_t* sec_hand;
} screen_data_t;

static void rotate(lv_obj_t* hand, int32_t value) {
    lv_obj_set_style_transform_rotation(hand, value, 0);
}

static void update_time(lv_observer_t* obs, lv_subject_t* sub) {
    time_t now = time(NULL);
    struct tm datetime;
    localtime_r(&now, &datetime);
    screen_data_t* data = lv_observer_get_user_data(obs);
    lv_label_set_text_fmt(data->lbl_time, "%02d:%02d", datetime.tm_hour, datetime.tm_min);
    rotate(data->hr_hand, 3600/(60*12)*(datetime.tm_hour*60+datetime.tm_min));
    rotate(data->min_hand, 3600/60*datetime.tm_min);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, data->sec_hand);
    lv_anim_set_duration(&a, 60000/60*(60-datetime.tm_sec));
    lv_anim_set_values(&a, 3600/60*datetime.tm_sec, 3600);
    lv_anim_set_exec_cb(&a, rotate);
    lv_anim_start(&a);
}


static lv_obj_t* clock_hand_create(lv_obj_t* parent, int w, int h, int offset, lv_color_t c) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, -h/2-offset);
    lv_obj_set_style_transform_pivot_x(obj, w/2, 0);
    lv_obj_set_style_transform_pivot_y(obj, h+offset, 0);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, c, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    data->lbl_time = lv_label_create(scr);
    lv_obj_align(data->lbl_time, LV_ALIGN_CENTER, 0, -60);

    data->hr_hand = clock_hand_create(scr, 10, 60, 15, lv_palette_main(LV_PALETTE_ORANGE));
    data->min_hand = clock_hand_create(scr, 6, 80, 15, lv_palette_main(LV_PALETTE_GREEN));
    data->sec_hand = clock_hand_create(scr, 8, 8, 86, lv_palette_main(LV_PALETTE_RED));

    lv_subject_add_observer_obj(&datetime_sub, update_time, scr, data);
    lv_obj_add_event_cb(get_default_left_button(), back_to_home_app, LV_EVENT_CLICKED, data);
}

static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
}

static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};


lv_app_t clock_app = {
    .name = {"Clock", "时钟"},
    .icon_img = &clock_icon_40x40,
    .icon_font = "🕒",
    .screen = &screen,
};

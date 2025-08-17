#include "apps.h"

// 定义屏幕打开时的临时变量
typedef struct {
    lv_style_t lbl_style;
    lv_obj_t* label;
} screen_data_t;

// 定义在 app 关闭时任然需要存在的永久变量
static int click_count;
static int rotate_count;

static void open_screen(lv_obj_t* scr, screen_data_t* data);
static void close_screen(screen_data_t* data);
static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};


// 定义 app 
lv_app_t example_app = {
    .name = {"Example", "例子"},
    .icon_img = &plus_icon_40x40,
    .icon_font = "🔲",
    .screen = &screen,
};


static void button_event_handler(lv_event_t * e) {
    lv_obj_t* target = lv_event_get_target(e);
    screen_data_t* data = lv_app_get_screen_data();

    if(target == get_default_right_button()) {
        lv_label_set_text_fmt(data->label, "click: %d\nrotate: %d", ++click_count, rotate_count);
    } else if(target == get_default_left_button()) {
        lv_app_open(&home_app);
    }
}

static void encoder_cb(int inc) {
    rotate_count += inc;
    screen_data_t* data = lv_app_get_screen_data();
    lv_label_set_text_fmt(data->label, "click: %d\nrotate: %d", click_count, rotate_count);
}

/*
    屏幕打开时由系统分配给屏幕对象 scr 和内存空间 data,
    开发者自行在屏幕里添加 UI 元素，初始化临时变量 data
*/
static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    lv_style_init(&data->lbl_style);
    lv_style_set_align(&data->lbl_style, LV_ALIGN_CENTER);

    data->label = lv_label_create(scr);
    lv_obj_add_style(data->label, &data->lbl_style, 0);
    lv_label_set_text_fmt(data->label, "click: %d\nrotate: %d", click_count, rotate_count);

    lv_obj_add_event_cb(get_default_left_button(), button_event_handler, LV_EVENT_CLICKED, data);
    lv_obj_add_event_cb(get_default_right_button(), button_event_handler, LV_EVENT_CLICKED, data);

    encoder_indev_set_cb(encoder_cb);
}


/*
    屏幕关闭后屏幕对象及其子元素会由系统销毁，并释放屏幕的临时变量 data.
    如果有其他需要手动释放的资源，或需要保存的永久变量，在 close 回调函数里完成
*/
static void close_screen(screen_data_t* data) {
    lv_style_reset(&data->lbl_style);
    encoder_indev_set_cb(NULL);
    lv_obj_remove_event(get_default_right_button(), LV_EVENT_ALL);
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
}
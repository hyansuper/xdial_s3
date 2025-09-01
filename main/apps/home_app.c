#include "apps.h"

static const lv_app_t* app_list[] = {&clock_app, &settings_app, &player_app, &crypto_app, &ai_chat_app, 
                                    &example_app, &example_app, &example_app, &example_app};

typedef struct {
    lv_obj_t* lbl_name;
    lv_group_t* grp;
    lv_style_t icon_focused;
} screen_data_t;

static int app_id;

static void grp_focus_cb(lv_group_t* grp) {
    lv_obj_t* icon = lv_group_get_focused(grp);
    screen_data_t* data = lv_app_get_screen_data();
    app_id = (int)lv_obj_get_user_data(icon);
    lv_label_set_text_static(data->lbl_name, app_list[app_id]->name[lv_app_get_lang()]);
}

static void app_selected_cb(lv_event_t * e) {
    lv_app_open(app_list[app_id]);
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    data->grp = lv_group_create();

    lv_style_t* st = &data->icon_focused;
    lv_style_init(st);
    lv_style_set_border_width(st, 4);
    lv_style_set_border_color(st, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_radius(st, LV_RADIUS_CIRCLE);

    lv_obj_t* obj = data->lbl_name = lv_label_create(scr);
    lv_obj_center(obj);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, 0);

    int app_num = sizeof(app_list)/sizeof(lv_app_t*);
    int r = 92;
    int ang = 90;
    int ang_step = 360/ (app_num);
    for(int i=0; i<app_num; i++, ang+=ang_step) {
        lv_app_t* app = app_list[i];
        lv_obj_t* icon = lv_image_create(scr);
        lv_obj_add_style(icon, &data->icon_focused, LV_STATE_FOCUSED);
        lv_obj_align(icon, LV_ALIGN_CENTER, (r*lv_trigo_cos(ang)) >> LV_TRIGO_SHIFT, (r*lv_trigo_sin(ang)) >> LV_TRIGO_SHIFT);
        lv_image_set_src(icon, app->icon_img);

        lv_obj_set_user_data(icon, i);
        lv_group_add_obj(data->grp, icon);

        if(app_id == i) {
            lv_group_set_focus_cb(data->grp, grp_focus_cb);
            lv_group_focus_obj(icon);
        }
    }

    encoder_indev_set_group(data->grp);
    lv_obj_add_event_cb(get_default_right_button(), app_selected_cb, LV_EVENT_CLICKED, data);
}

static void close_screen(screen_data_t* data) {
    encoder_indev_set_group(NULL);
    lv_group_delete(data->grp);
    lv_style_reset(&data->icon_focused);
    lv_obj_remove_event(get_default_right_button(), LV_EVENT_ALL);
}

static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};


lv_app_t home_app = {
    .name = {"Home", "主界面"},
    .icon_img = NULL,
    .icon_font = "🏠",
    .screen = &screen,
};

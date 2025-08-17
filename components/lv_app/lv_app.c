#include "lv_app.h"
#include "esp_log.h"

static const char* const TAG = "lv_app";

static lv_app_t* curr_app, *prev_app;
lv_app_changed_cb_t lv_app_changed_cb;

struct _close_scr_t{
    lv_screen_cb_t close_cb;
    uint8_t screen_data[];
};

static void scr_unload_cb(lv_event_t* e) {
    struct _close_scr_t* close_scr = lv_event_get_user_data(e);
    if(close_scr->close_cb)
        close_scr->close_cb((void*)close_scr->screen_data);
    lv_free(close_scr);
}

static void lv_app_load_screen(lv_app_t* app, lv_screen_t* new) {
    app->screen = new;
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    app->screen_data = NULL;
    if(new->screen_data_size || new->close) {
        struct _close_scr_t* close_scr = lv_calloc(1, sizeof(struct _close_scr_t) + new->screen_data_size);
        if(close_scr == NULL) {
            ESP_LOGE(TAG, "no mem for lv_screen_data");
            return;
        }
        close_scr->close_cb = new->close;
        app->screen_data = (void*) close_scr->screen_data;
        lv_obj_add_event_cb(scr, scr_unload_cb, LV_EVENT_SCREEN_UNLOADED, close_scr);
    }
    new->open(scr, app->screen_data);

    #ifdef PC_SIM
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    #endif
    lv_screen_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true /*auto delete old screen*/);
}

void lv_app_refresh() {
	lv_app_load_screen(curr_app, curr_app->screen);
}

void lv_app_set_screen(lv_screen_t* screen) {
	if(screen != curr_app->screen) {
		lv_app_load_screen(curr_app, screen);
	}
}

void lv_app_open(lv_app_t* app) {
	if(app && app!=curr_app) {
        prev_app = curr_app;
        curr_app = app;
        lv_app_load_screen(app, app->screen);

        #ifndef PC_SIM
        if (lv_app_changed_cb)
        	lv_app_changed_cb(app);
        #endif
    }
}

// 返回上一个app, 只能返回一层
void lv_app_open_prev() {
	lv_app_open(prev_app);
    prev_app = NULL;
}

lv_app_t* lv_app_get_current() {
	return curr_app;
}

void* lv_app_get_data() {
	if(curr_app) return curr_app->screen_data;
	return NULL;
}


const lv_point_t* app_get_default_button_points();
lv_obj_t* app_get_default_right_button();
lv_obj_t* app_get_default_left_button();

void lv_app_init() {

}
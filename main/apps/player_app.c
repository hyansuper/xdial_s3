/*

这个例子结合 esp_gmf，演示如何播放音乐
参考 https://github.com/espressif/esp-gmf/blob/main/gmf_examples/basic_examples/pipeline_play_embed_music/main/play_embed_music.c
当然，也可以用更高级的组件 esp_gmf_simple_player
*/
#include "apps.h"
#include "audio_manager.h"

typedef struct {
    lv_obj_t* button, *label;
    esp_gmf_pipeline_handle_t pipe;
} screen_data_t;

static esp_err_t _pipeline_event(esp_gmf_event_pkt_t *event, screen_data_t* data)
{
    if (event->sub == ESP_GMF_EVENT_STATE_FINISHED) {
        // audio_mgr_release_pipeline(data->pipe); // can't delete pipe task in it's own event callback
        WITH_LV_LOCK({ lv_label_set_text_static(data->label, LV_SYMBOL_PLAY); });
    }
    return 0;
}

static esp_gmf_pipeline_handle_t create_player_pipeline(void* user_data) {
    esp_gmf_pipeline_handle_t pipe;
    const char *name[] = {"aud_dec", "aud_bit_cvt", "aud_rate_cvt"};
    pipe = audio_mgr_create_ready_playback_pipeline("io_embed_flash", name, sizeof(name)/sizeof(char*), NULL);
    esp_gmf_pipeline_set_in_uri(pipe, embed_tone_url[0]);
    esp_gmf_io_embed_flash_set_context(ESP_GMF_PIPELINE_GET_IN_INSTANCE(pipe), embed_tone_info, 1);
    esp_gmf_element_handle_t dec_el = NULL;
    esp_gmf_pipeline_get_el_by_name(pipe, "aud_dec", &dec_el);
    esp_gmf_info_sound_t info = {0};
    esp_gmf_audio_helper_get_audio_type_by_uri(embed_tone_url[0], &info.format_id);
    esp_gmf_audio_dec_reconfig_by_sound_info(dec_el, &info);

    esp_gmf_pipeline_set_event(pipe, _pipeline_event, user_data);
    return pipe;
}


static void right_button_clicked(lv_event_t * e) {
    screen_data_t* data = lv_app_get_screen_data();
    if(0==strcmp(LV_SYMBOL_PLAY, lv_label_get_text(data->label))) {
        // if pipeline playing is already finished, resume() will fail
        if(ESP_GMF_ERR_OK!= esp_gmf_pipeline_resume(data->pipe)) {
            audio_mgr_release_pipeline(data->pipe);
            data->pipe = create_player_pipeline(data);
            esp_gmf_pipeline_run(data->pipe);
        }
        lv_label_set_text_static(data->label, LV_SYMBOL_PAUSE);
        
    } else {
        esp_gmf_pipeline_pause(data->pipe);
        lv_label_set_text_static(data->label, LV_SYMBOL_PLAY);
        
    }
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    data->button = lv_button_create(scr);
    lv_obj_set_style_bg_color(data->button, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_center(data->button);

    data->label = lv_label_create(data->button);
    lv_obj_center(data->label);
    lv_obj_set_style_text_font(data->label, &lv_font_montserrat_22, 0);

    // 因为PCB布线问题，左边按键容易误触，特别是播放声音的时候，因此改为长按才能退出
    lv_obj_add_event_cb(get_default_left_button(), back_to_home_app, LV_EVENT_LONG_PRESSED, data);

    lv_obj_add_event_cb(data->button, right_button_clicked, LV_EVENT_CLICKED, data);
    button_indev_set_right_button_point(120, 120); // 屏幕中间，就是播放按钮的位置

    data->pipe = create_player_pipeline(data);
    esp_gmf_pipeline_run(data->pipe);

    lv_label_set_text_static(data->label, LV_SYMBOL_PAUSE);
}


static void close_screen(screen_data_t* data) {
    // lv_obj_remove_event(data->button, LV_EVENT_ALL); // 因为 data->button 是屏幕的子元素，关闭时会被自动释放，其回调函数也会自动失效
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
    button_indev_set_default_button_points(); // 因为先前将右键坐标改到屏幕中间，这里恢复到默认位置
    audio_mgr_release_pipeline(data->pipe);   
}

static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};


lv_app_t player_app = {
    .name = {"Player", "播放"},
    .icon_img = &play_icon_40x40,
    .icon_font = "▶",
    .screen = &screen,
};

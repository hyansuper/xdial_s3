/*

这个例子演示如何显示获取的网络数据。
网络数据的获取见 main/periodic_data_update/
数据与 UI 的绑定使用了 lvgl 的 observer 

*/

#include "apps.h"
#include "crypto_coin.h"

typedef struct {
    lv_obj_t* price;
    lv_obj_t* icon;
} screen_data_t;


static void open_screen(lv_obj_t* scr, screen_data_t* data);
static void close_screen(screen_data_t* data);
static const lv_screen_t screen = {
    .open = open_screen,
    .close = close_screen,
    .screen_data_size = sizeof(screen_data_t),
};

// 定义 app 
lv_app_t crypto_app = {
    .name = {"Crypto", "加密货币"},
    .icon_img = &bitcoin_icon_40x40,
    .icon_font = "₿",
    .screen = &screen,
};

static void update_price(lv_observer_t* obs, lv_subject_t* sub) {
    screen_data_t* data = lv_observer_get_user_data(obs);
    for(int i=0; i<crypto_coin_list.coin_num; i++) {
        crypto_coin_t* coin = &crypto_coin_list.coins[i];
        if(strcmp(coin->symbol, "BTC")==0) {
            int price_integral = (int)coin->price;
            int price_digit = ((int)(coin->price * 100)) %100;
            lv_label_set_text_fmt(data->price, "$ %d.%02d", price_integral, price_digit);   
        }
    }
}

static void open_screen(lv_obj_t* scr, screen_data_t* data) {
    data->icon = lv_image_create(scr);
    lv_image_set_src(data->icon, &bitcoin_icon_40x40);
    lv_obj_align(data->icon, LV_ALIGN_CENTER, 0, -30);

    data->price = lv_label_create(scr);
    lv_obj_set_style_text_font(data->price, &lv_font_montserrat_22, 0);
    lv_obj_align(data->price, LV_ALIGN_CENTER, 0, 30);

    lv_subject_add_observer_obj(&crypto_sub, update_price, scr, data);
    lv_obj_add_event_cb(get_default_left_button(), back_to_home_app, LV_EVENT_LONG_PRESSED, data);
}


static void close_screen(screen_data_t* data) {
    lv_obj_remove_event(get_default_left_button(), LV_EVENT_ALL);
}
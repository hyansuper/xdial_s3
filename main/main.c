#include "esp_log.h"
#include "esp_event.h"
#include "esp_spiffs.h"
#include "board.h"
#include "lv_port.h"
#include "apps.h"
#include "lv_observers.h"
#include "server.h"
#include "audio_manager.h"
#include "periodic_data_update.h"
#include "wifi_manager.h"
#include "ai_chat.h"
#include "kv.h"
#include "sntp_srv.h"

static esp_err_t init_spiffs() {
	const esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 5,
        .format_if_mount_failed = false
    };
    return esp_vfs_spiffs_register(&spiffs_conf);
}

static void datetime_minutely_update(struct tm* dt) {
	WITH_LV_LOCK({ lv_subject_notify(&datetime_sub); });
	periodic_data_update();
}

static void wifi_state_changed(wifi_mgr_state_t* s) {
	WITH_LV_LOCK({ lv_subject_notify(&wifi_sub); });
}


static void bind_observers() {
	static const void* pairs[] = {&datetime_sub, &sntp_datetime, 
								&wifi_sub, &wifi_mgr_state,
								&crypto_sub, &crypto_coin_list};

	for(int i=0; i<sizeof(pairs)/sizeof(void*); i+=2) 
		lv_subject_init_pointer(pairs[i], pairs[i+1]);
}

static void wifi_prov_cb(wifi_mgr_state_t* state) {
	if(state->connected) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    }
}

void app_main(void) {
	board_init(); // 初始化板子相关的硬件
	lv_port();   // 初始化 lvgl 
	lcd_set_brightness(20);

	lv_obj_t* label;
	WITH_LV_LOCK({
		label = lv_label_create(lv_screen_active());
		lv_obj_center(label);
		lv_label_set_text_static(label, APP_NAME " - " APP_VER);
	});
	vTaskDelay(pdMS_TO_TICKS(1000));

	// 启动网路
	ESP_ERROR_CHECK(esp_netif_init() || esp_event_loop_create_default());
	wifi_mgr_init();

	if(ESP_OK!= wifi_mgr_sta(wifi_state_changed)) {
		wifi_mgr_prov(wifi_prov_cb);
		WITH_LV_LOCK({ 
			lv_label_set_text_fmt(label, LV_APP_LANG(
							"1. Connect to hotspot\n    %s\n2. Visit %s\n    with browser", 
							"1. 连接 WiFi 热点\n    %s\n2. 用浏览器访问网址\n    %s"),
                             wifi_mgr_hostname(), wifi_mgr_ipstr());
		 });
		goto end;
    
    }

    ESP_ERROR_CHECK(init_spiffs());
	sntp_srv_init("UTC-8", datetime_minutely_update);
	server_start();
	ESP_ERROR_CHECK(audio_mgr_init());
	vTaskDelay(pdMS_TO_TICKS(1000));
	ESP_ERROR_CHECK(kv_init());
	ai_chat_init();

	WITH_LV_LOCK({
		bind_observers();
		lv_app_init();
		lv_app_open(&home_app);
	});
end:
    vTaskDelete(NULL);
}
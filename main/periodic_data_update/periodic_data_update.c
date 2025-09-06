#include "periodic_data_update.h"
#include "esp_check.h"
#include "task_util.h"
#include "http_client_util.h"
#include "lv_port.h"

static const char* const TAG = "data_update";
static TaskHandle_t task;

static time_t last_crypto_update;
static uint32_t crypto_update_period = 60* 2; // update every 2 min

#define RAW_BUF_SIZE (1024*2)
#define UNZIP_BUF_SIZE (1024*4)

static void run_task(void* arg) {
	esp_http_client_handle_t client = http_client_util_create();
	void* raw_buf = heap_caps_malloc(RAW_BUF_SIZE, MALLOC_CAP_SPIRAM);
	void* unzip_buf = heap_caps_malloc(UNZIP_BUF_SIZE, MALLOC_CAP_SPIRAM);
	if(!(client && raw_buf && unzip_buf)) goto cleanup;

	time_t now = time(NULL);

	if(now-last_crypto_update>=crypto_update_period && ESP_OK==crypto_coin_fetch(client, raw_buf, RAW_BUF_SIZE, unzip_buf, UNZIP_BUF_SIZE, "90,80")) {
		last_crypto_update = now;
	}

	// if(now-last_weather_update>=weather_update_period && ESP_OK==weather_data_fetch(...) {
	// 	last_weather_update = now;
	// }

	WITH_LV_LOCK({
		if(now==last_crypto_update) 
			lv_subject_notify(&crypto_sub);

		// if(now==last_weather_update)
		// 	lv_subject_notify(&weather_sub);
	});
	

cleanup:
	http_client_util_delete(client);
	RELEASE(raw_buf);
	RELEASE(unzip_buf);
	task = NULL; // set task to NULL before task_delete()
	capped_task_delete(NULL); // anything after this line is not effective
}

static bool update_required() {
	time_t now = time(NULL);
	return (now-last_crypto_update>=crypto_update_period) 
			// || (now-last_weather_update>=weather_update_period) 
			;
}
void periodic_data_update() {
	if(task==NULL && update_required()) {
		capped_task_config_t conf = CAPPED_TASK_CONFIG_SPI(4096, 2);
		capped_task_create(&task, TAG, run_task, NULL, &conf);
	}
}
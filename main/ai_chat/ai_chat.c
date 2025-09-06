#include "ai_chat.h"
#include "audio_manager.h"
#include "esp_audio_simple_player_advance.h"
#include "esp_gmf_new_databus.h"
#include "esp_gmf_fifo.h"
#include "esp_gmf_afe.h"
#include "esp_gmf_rate_cvt.h"
#include "esp_opus_enc.h"
#include "esp_opus_dec.h"
#include "esp_audio_types.h"
#include "esp_gmf_audio_enc.h"
#include "esp_fourcc.h"
#include "board.h"
#include "apps.h"

static const char* const TAG = "ai_chat";

typedef struct {
    esp_gmf_fifo_handle_t         fifo;
    esp_gmf_pipeline_handle_t     pipe;
} recorder_t;

typedef struct {
    esp_gmf_db_handle_t fifo;
    esp_gmf_pipeline_handle_t pipe;
} playback_t;

static playback_t playback;
static recorder_t recorder;
static esp_asp_handle_t player;
xz_chat_t* chat;
char* ai_chat_activation_msg;

static int playback_inport_acquire_read(void *handle, esp_gmf_data_bus_block_t *blk, int wanted_size, int block_ticks){
    esp_gmf_data_bus_block_t _blk = {0};
    int ret = esp_gmf_db_acquire_read(playback.fifo, &_blk, wanted_size, block_ticks);
    if (ret < 0) {
        ESP_LOGE(TAG, "Fifo acquire read failed (0x%x)", ret);
        return ESP_FAIL;
    }
    if(_blk.valid_size > wanted_size) {
        ESP_LOGE(TAG, "acceptable size less than fifo block");
        esp_gmf_db_release_read(playback.fifo, &_blk, block_ticks);
        return ESP_FAIL;
    }
    memcpy(blk->buf, _blk.buf, _blk.valid_size);
    blk->valid_size = _blk.valid_size;
    esp_gmf_db_release_read(playback.fifo, &_blk, block_ticks);
    return ESP_GMF_ERR_OK;
}

static int playback_inport_release_read(void *handle, esp_gmf_data_bus_block_t *blk, int block_ticks){
    return ESP_GMF_ERR_OK;
}
static void playback_init_and_run() {
	esp_err_t err = esp_gmf_db_new_fifo(5 /*size*/, 1 /*unsused param*/, &playback.fifo);
    ESP_ERROR_CHECK(err);
	
	const char *name[] = { "aud_dec", /*"aud_bit_cvt",*/ "aud_rate_cvt", /*"aud_ch_cvt"*/};
    audio_mgr_new_playback_pipeline(NULL, name, sizeof(name)/sizeof(char*), &playback.pipe);
    assert(playback.pipe);
    // set pipe input
    esp_gmf_port_handle_t in_port = NEW_ESP_GMF_PORT_IN_BYTE(playback_inport_acquire_read, playback_inport_release_read, NULL, NULL, 4096, portMAX_DELAY);
    esp_gmf_pipeline_reg_el_port(playback.pipe, "aud_dec", ESP_GMF_IO_DIR_READER, in_port);

    esp_gmf_element_handle_t el;
    esp_gmf_pipeline_get_el_by_name(playback.pipe, "aud_rate_cvt", &el);
    esp_gmf_rate_cvt_set_dest_rate(el, speaker_get_sample_rate());

    esp_gmf_info_sound_t info = {
        .sample_rates = 24000,
        .channels = 1,
        .bits = 16,
        .format_id = ESP_FOURCC_OPUS,
    };
    esp_gmf_pipeline_report_info(playback.pipe, ESP_GMF_INFO_SOUND, &info, sizeof(info));
    esp_gmf_task_cfg_t task_cfg = DEFAULT_ESP_GMF_TASK_CONFIG();
    task_cfg.name="playback_thread";
    task_cfg.thread.stack = 40*1024;
    task_cfg.thread.stack_in_ext = true;
    ESP_ERROR_CHECK(audio_mgr_pipeline_bind_task(playback.pipe, &task_cfg));
	esp_gmf_pipeline_run(playback.pipe);
}

static esp_err_t play_ding() {
	return esp_audio_simple_player_run_to_end(player, "file://spiffs/dingding.wav", NULL);
}

static void afe_event_cb(esp_gmf_obj_handle_t obj, esp_gmf_afe_evt_t *event, void *user_data) {
    switch (event->type) {
        case ESP_GMF_AFE_EVT_WAKEUP_START: {
            if(!xz_chat_is_in_session(chat)) {
            	// WITH_LV_LOCK({
	            //     if(&ai_chat_app == lv_app_get_current()) {

	            //     } else {
	            //     	lv_app_open(&ai_chat_app);
	            //     }
	            // });
                play_ding();
                xz_chat_new_session(chat);
            }
            break;
        }
    	default:;
    }
}


static int recorder_outport_acquire_write(void *handle, esp_gmf_data_bus_block_t *blk, int wanted_size, int block_ticks){
    return wanted_size;
}
static int recorder_outport_release_write(void *handle, esp_gmf_data_bus_block_t *blk, int block_ticks){
    esp_gmf_data_bus_block_t _blk = {0};
    int ret = esp_gmf_fifo_acquire_write(recorder.fifo, &_blk, blk->valid_size, block_ticks);
    if (ret < 0) {
        ESP_LOGE(TAG, "%s|%d, Fifo acquire write failed, ret: %d", __func__, __LINE__, ret);
        return ESP_FAIL;
    }
    memcpy(_blk.buf, blk->buf, blk->valid_size);

    _blk.valid_size = blk->valid_size;
    ret = esp_gmf_fifo_release_write(recorder.fifo, &_blk, block_ticks);
    if (ret != ESP_GMF_ERR_OK) {
        ESP_LOGE(TAG, "Fifo release write failed");
        return ESP_FAIL;
    }
    return blk->valid_size;
}


static void recorder_init_and_run() {
    esp_err_t err = esp_gmf_fifo_create(5 /*size*/, 1 /*unsused param*/, &recorder.fifo);
    ESP_ERROR_CHECK(err);

    const char *recorder_elements[] = {"aud_rate_cvt", "ai_afe", "aud_enc"};
    audio_mgr_new_recorder_pipeline(recorder_elements, sizeof(recorder_elements)/sizeof(char*), NULL, &recorder.pipe);
    assert(recorder.pipe);
    esp_gmf_port_handle_t out_port = NEW_ESP_GMF_PORT_OUT_BYTE(recorder_outport_acquire_write, recorder_outport_release_write, NULL, NULL, 4096, portMAX_DELAY);
    esp_gmf_pipeline_reg_el_port(recorder.pipe, "aud_enc", ESP_GMF_IO_DIR_WRITER, out_port);

    // 因为小智接收到的是24K的采样率，而 es8311 输入和输出要设置相同的采样率, 因此选择 24K。
    // 但 唤醒检测的 ai_afe 接受16K 的采样率，因此要将麦克风的24K转为16K在给 ai_afe，最终发给小智服务器的是 16K
    esp_gmf_info_sound_t info = {
        .sample_rates = microphone_get_sample_rate(),
        .channels = 1,
        .bits = 16,
    };
    esp_gmf_pipeline_report_info(recorder.pipe, ESP_GMF_INFO_SOUND, &info, sizeof(info));

    esp_gmf_element_handle_t el;
    esp_gmf_pipeline_get_el_by_name(recorder.pipe, "aud_rate_cvt", &el);
    esp_gmf_rate_cvt_set_dest_rate(el, 16000);

    esp_gmf_pipeline_get_el_by_name(recorder.pipe, "ai_afe", &el);
    esp_gmf_afe_set_event_cb(el, afe_event_cb, NULL);
    
    const esp_opus_enc_config_t opus_enc_cfg = {
        .sample_rate        = ESP_AUDIO_SAMPLE_RATE_16K,          
        .channel            = ESP_AUDIO_MONO,                    
        .bits_per_sample    = ESP_AUDIO_BIT16,                   
        .bitrate            = 17000, // 小智的实测是取 17000，改大点也行，但发送的数据量变大
        .frame_duration     = ESP_OPUS_ENC_FRAME_DURATION_60_MS, 
        .application_mode   = ESP_OPUS_ENC_APPLICATION_VOIP,     
        .complexity         = 5, // 这个值在小智里如果开启 AEC 则取 0
        .enable_fec         = false,                             
        .enable_dtx         = true,                             
        .enable_vbr         = true,                             
    };
    esp_audio_enc_config_t opus_cfg = {
        .type = ESP_AUDIO_TYPE_OPUS,
        .cfg_sz = sizeof(esp_opus_enc_config_t),
        .cfg = &opus_enc_cfg,
    };
    esp_gmf_pipeline_get_el_by_name(recorder.pipe, "aud_enc", &el);
    esp_gmf_audio_enc_reconfig(el, &opus_cfg);

    esp_gmf_task_cfg_t task_cfg = DEFAULT_ESP_GMF_TASK_CONFIG();
    task_cfg.name = "recorder_thread";
    task_cfg.thread.stack = 40*1024;
    task_cfg.thread.stack_in_ext = true;
    ESP_ERROR_CHECK( audio_mgr_pipeline_bind_task(recorder.pipe, &task_cfg));
	esp_gmf_pipeline_run(recorder.pipe);
}

static int player_out_data_callback(uint8_t *data, int data_size, void *ctx){
    esp_codec_dev_write(speaker_get_codec_dev(), data, data_size);
    return 0;
}

static esp_err_t simple_player_init() {
	const esp_asp_cfg_t cfg = {
        .in.cb = NULL,
        .in.user_ctx = NULL,
        .out.cb = player_out_data_callback,
        .out.user_ctx = NULL,
        .task_prio = 5,
        // .task_stack_in_ext = 1,
    };
    return esp_audio_simple_player_new(&cfg, &player);
}

static void xz_chat_on_audio(uint8_t *data, int len, xz_chat_t* chat) {
    esp_gmf_data_bus_block_t blk = {0};
    int ret = esp_gmf_db_acquire_write(playback.fifo, &blk, len, portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to acquire write to playback FIFO (0x%x)", ret);
        return;
    }
    int bytes_to_copy = (len < blk.buf_length) ? len : blk.buf_length;
    memcpy(blk.buf, data, bytes_to_copy);
    blk.valid_size = bytes_to_copy;
    
    ret = esp_gmf_db_release_write(playback.fifo, &blk, portMAX_DELAY);
}

static void return_blk_to_recorder_fifo(void* blk){
    esp_gmf_fifo_release_read(recorder.fifo, (esp_gmf_data_bus_block_t*)blk, portMAX_DELAY);
    free(blk);
}

static esp_err_t xz_chat_read_audio(xz_tx_audio_pck_t* audio, xz_chat_t* chat) {
    esp_gmf_data_bus_block_t* blk = calloc(1, sizeof(esp_gmf_data_bus_block_t));
    if(blk==NULL) return ESP_ERR_NO_MEM;
    int ret = esp_gmf_fifo_acquire_read(recorder.fifo, blk, 1024/*unused param*/, portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to acquire read from recorder FIFO (0x%x)", ret);
        return ESP_FAIL;
    }
    audio->buf = blk->buf;
    audio->len = blk->valid_size;
    audio->user_data = blk;
    audio->release_cb = return_blk_to_recorder_fifo;
    return ESP_OK;
}


static void xz_chat_on_event(xz_chat_event_t event, xz_chat_event_data_t *event_data, xz_chat_t* chat) {

    if(event==XZ_EVENT_VERSION_CHECK_RESULT) {
        if(event_data->version_check_err ==0) {
            if(event_data->parsed_response->require_activation) {

                // tell the user they must visit xiaozhi.me and register device with activation code
                ESP_LOGI(TAG, "********************\n\n%s\n\n********************", event_data->parsed_response->activation.message);
                ai_chat_activation_msg = strdup( event_data->parsed_response->activation.message);
            } else {
            	xz_chat_start(chat);
            }
        }

    } else if(event==XZ_EVENT_ACTIVATION_CHECK_RESULT) {
        if(event_data->activation_check_err) { 

        } else {
        	xz_chat_start(chat);
        }

    } else if(event==XZ_EVENT_STARTED) {
        // xz_chat is started and ready to chat
        ESP_LOGI(TAG, "xiaozhi started");

    } 
}

void ai_chat_init() {
	xz_board_info_load();
	xz_chat_config_t chat_conf = XZ_CHAT_CONFIG_DEFAULT(xz_chat_read_audio, xz_chat_on_event, xz_chat_on_audio);
    chat = xz_chat_init(&chat_conf);
    assert(chat);  

    recorder_init_and_run();
    playback_init_and_run();
    simple_player_init();
    
    xz_chat_version_check(chat, NULL);
}

void ai_chat_deinit() {
	xz_chat_destroy(chat);
	esp_audio_simple_player_stop(player);
	esp_audio_simple_player_destroy(player);
	audio_mgr_release_pipeline(playback.pipe);
	audio_mgr_release_pipeline(recorder.pipe);
    esp_gmf_fifo_destroy(recorder.fifo);
}
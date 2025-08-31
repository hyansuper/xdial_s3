#include "audio_manager.h"
#include "board.h"
#include "esp_check.h"

static const char* const TAG = "audio_mgr";

static esp_gmf_pool_handle_t pool;

esp_gmf_pool_handle_t audio_mgr_get_pool() {
	return pool;
}

esp_gmf_err_t audio_mgr_init() {
	esp_err_t ret = ESP_GMF_ERR_OK;
    ESP_RETURN_ON_ERROR(esp_gmf_pool_init(&pool), TAG, "init pool");
	ESP_GOTO_ON_ERROR(gmf_loader_setup_all_defaults(pool), err, TAG, "load elements");
    return ESP_GMF_ERR_OK;
err:
	audio_mgr_deinit();
	return ret;
}

esp_gmf_err_t audio_mgr_deinit() {
	if(pool == NULL) return ESP_GMF_ERR_OK;
	esp_err_t err = gmf_loader_teardown_all_defaults(pool) ||
					esp_gmf_pool_deinit(pool);
	if(err) return err;
    pool = NULL;
    return ESP_GMF_ERR_OK;
}


esp_gmf_err_t audio_mgr_new_playback_pipeline(const char* in_name, const char* el_name, int num_of_el_name, esp_gmf_pipeline_handle_t* pipeline) {
	esp_gmf_err_t err = esp_gmf_pool_new_pipeline(pool, in_name, el_name, num_of_el_name, "io_codec_dev", pipeline);
	if(err) return err;
    esp_gmf_io_codec_dev_set_dev(ESP_GMF_PIPELINE_GET_OUT_INSTANCE((*pipeline)), speaker_get_codec_dev());
	return ESP_GMF_ERR_OK;
}

esp_gmf_err_t audio_mgr_new_recoder_pipeline(const char* el_name[], int num_of_el_name, const char* out_name, esp_gmf_pipeline_handle_t* pipeline) {
	esp_gmf_err_t err = esp_gmf_pool_new_pipeline(pool, "io_codec_dev", el_name, num_of_el_name, out_name, pipeline);
	if(err) return err;
    esp_gmf_io_codec_dev_set_dev(ESP_GMF_PIPELINE_GET_IN_INSTANCE((*pipeline)), microphone_get_codec_dev());
	return ESP_GMF_ERR_OK;
}

esp_gmf_pipeline_handle_t audio_mgr_create_ready_playback_pipeline(const char* in_name, const char* el_name, int num_of_el_name, esp_gmf_task_cfg_t* task_cfg) {
	esp_gmf_pipeline_handle_t pipe;
	esp_gmf_err_t err = audio_mgr_new_playback_pipeline(in_name, el_name, num_of_el_name, &pipe);
	if(err) return NULL;

	esp_gmf_task_handle_t work_task = NULL;
	esp_gmf_task_cfg_t def_task_cfg = DEFAULT_ESP_GMF_TASK_CONFIG();
	if(task_cfg==NULL) task_cfg = &def_task_cfg;
    err = esp_gmf_task_init(task_cfg, &work_task);
    ESP_GMF_RET_ON_NOT_OK(TAG, err, { return NULL; }, "Failed to create pipeline task");
    esp_gmf_pipeline_bind_task(pipe, work_task);
    esp_gmf_pipeline_loading_jobs(pipe);
    return pipe;
}

esp_gmf_err_t audio_mgr_delete_pipeline(esp_gmf_pipeline_handle_t pipe) {
	ESP_GMF_NULL_CHECK(TAG, pipe, return ESP_GMF_ERR_INVALID_ARG);
	if(pipe->thread) {
		esp_gmf_err_t err = esp_gmf_task_deinit(pipe->thread);
		if(err) return err;
	}
    return esp_gmf_pipeline_destroy(pipe);
}
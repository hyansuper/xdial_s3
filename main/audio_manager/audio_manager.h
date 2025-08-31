#pragma once

#include <esp_codec_dev.h>
#include "esp_gmf_element.h"
#include "esp_gmf_pipeline.h"
#include "esp_gmf_pool.h"
#include "esp_gmf_io_embed_flash.h"	
#include "esp_gmf_audio_helper.h"
#include "esp_gmf_audio_dec.h"
#include "gmf_loader_setup_defaults.h"
#include "esp_gmf_io_codec_dev.h"

#include "embed_tone.h"


esp_gmf_err_t audio_mgr_init();
esp_gmf_err_t audio_mgr_deinit();
esp_gmf_pool_handle_t audio_mgr_get_pool();

esp_gmf_err_t audio_mgr_new_playback_pipeline(const char* in_name, const char* el_name, int num_of_el_name, esp_gmf_pipeline_handle_t* pipeline);
esp_gmf_err_t audio_mgr_new_recoder_pipeline(const char* el_name[], int num_of_el_name, const char* out_name, esp_gmf_pipeline_handle_t* pipeline);

esp_gmf_pipeline_handle_t audio_mgr_create_ready_playback_pipeline(const char* in_name, const char* el_name, int num_of_el_name, esp_gmf_task_cfg_t* task_cfg);


esp_gmf_err_t audio_mgr_delete_pipeline(esp_gmf_pipeline_handle_t pipe);
#pragma once
#include <esp_lcd_panel_io.h>

#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>

void lcd_init();
int32_t lcd_get_pwm_duty();
void lcd_set_pwm_duty(int32_t duty);
int lcd_get_brightness();
void lcd_set_brightness(int br);

esp_lcd_panel_io_handle_t lcd_get_panel_io_handle();
esp_lcd_panel_handle_t lcd_get_panel_handle();



esp_err_t audio_init();
void audio_deinit();
esp_err_t speaker_set_volume(int v);
esp_err_t speaker_enable(bool en);
esp_err_t microphone_enable(bool en);
int speaker_get_volume();
int speaker_get_sample_rate();
int microphone_get_sample_rate();
esp_codec_dev_handle_t get_speaker_codec_dev();
esp_codec_dev_handle_t get_microphone_codec_dev();

void input_init_gpio();

static inline void board_init() {
    input_init_gpio();
    lcd_init();
    audio_init();
}


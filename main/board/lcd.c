#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_gc9a01.h"

#include "board_config.h"
#include "board.h"

static const char* const TAG = "lcd";

static esp_lcd_panel_io_handle_t io_handle;
static esp_lcd_panel_handle_t panel_handle;
esp_lcd_panel_io_handle_t lcd_get_panel_io_handle() {
    return io_handle;
}
esp_lcd_panel_handle_t lcd_get_panel_handle() {
    return panel_handle;
}

#define BRIGTNESS_TO_PWM_DUTY(br)  (br* (DISPLAY_BACKLIGHT_FULL_DUTY/100/5))  // 太亮了，缩放 1/5

static int backlight_brightness;
int lcd_get_brightness() {
    return backlight_brightness;
}

void lcd_set_brightness(int br) {
    if(br<0) br=0;
    else if(br>100) br=100;
    if(backlight_brightness==br) return;
    backlight_brightness = br;
    lcd_set_pwm_duty(BRIGTNESS_TO_PWM_DUTY(br));
}

void lcd_set_pwm_duty(int32_t duty_cycle) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, DISPLAY_BACKLIGHT_PWM_CHANNEL, duty_cycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, DISPLAY_BACKLIGHT_PWM_CHANNEL);
}

int32_t lcd_get_pwm_duty() {
    return ledc_get_duty(LEDC_LOW_SPEED_MODE, DISPLAY_BACKLIGHT_PWM_CHANNEL);
}

static void lcd_init_pwm() {
    ESP_LOGI(TAG, "Initialize LCD backlight");
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = DISPLAY_BACKLIGHT_DUTY_RES_BIT,
        .timer_num        = DISPLAY_BACKLIGHT_PWM_TIMER,
        .freq_hz          = DISPLAY_BACKLIGHT_PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = DISPLAY_BACKLIGHT_PWM_CHANNEL,
        .timer_sel      = DISPLAY_BACKLIGHT_PWM_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_DISPLAY_BACKLIGHT,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}




void lcd_init() {
    lcd_init_pwm();

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = GPIO_DISPLAY_SPI_SCLK,
        .mosi_io_num = GPIO_DISPLAY_SPI_MOSI,
        .miso_io_num = GPIO_DISPLAY_SPI_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_SPI_MAX_TRANSFER,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(DISPLAY_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = GPIO_DISPLAY_SPI_DC,
        .cs_gpio_num = GPIO_DISPLAY_SPI_CS,
        .pclk_hz = DISPLAY_SPI_SCLK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        // .on_color_trans_done = notify_lvgl_flush_ready,
        // .user_ctx = display,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)DISPLAY_SPI_NUM, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_DISPLAY_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_LOGI(TAG, "Install GC9A01 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
}

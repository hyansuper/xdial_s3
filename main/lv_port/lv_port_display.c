#include "lv_port.h"
#include "lv_port_config.h"

#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#if LVGL_USE_DMA_BUFFER
#define BUF_ATTR DMA_ATTR
#else
#define BUF_ATTR
#endif

#if defined (CONFIG_SPI_MASTER_IN_IRAM)
#define SPI_CB_ATTR IRAM_ATTR
#else
#define SPI_CB_ATTR
#endif

#if LVGL_TASK_STACK_IN_SPIRAM
#define TASK_STACK_ATTR EXT_RAM_NOINIT_ATTR 
#else
#define TASK_STACK_ATTR 
#endif
TASK_STACK_ATTR static StackType_t lvgl_task_stack[LVGL_TASK_STACK];
static StaticTask_t lvgl_task_buffer;

#if LVGL_USE_DOUBLE_BUFFER
BUF_ATTR static lv_color_t buf[2][LVGL_DISPLAY_BUFFER_PIXELS];
#else
BUF_ATTR static lv_color_t buf[LVGL_DISPLAY_BUFFER_PIXELS];
#endif

static void lvgl_task_cb(void *pvParameters) {
    uint32_t delay_ms = 0;
    TickType_t wait_tick;
    while(1) {
        wait_tick = pdMS_TO_TICKS(delay_ms);
        if(wait_tick <1)
            wait_tick = 1;
        vTaskDelay(wait_tick);
        // lv_lock();
        delay_ms = lv_timer_handler(); 
        // lv_unlock();
        if(delay_ms > LVGL_MAX_UPDATE_INTERVAL_MS)
            delay_ms = LVGL_MAX_UPDATE_INTERVAL_MS;
    }
}

static uint32_t lvgl_tick_cb() {
    return esp_timer_get_time()/1000;
}

SPI_CB_ATTR static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_flush_ready(user_ctx);
    return false;
}

SPI_CB_ATTR static void lvgl_flush_cb(lv_disp_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) lv_display_get_user_data(drv);

    #if LVGL_SWAP_COLOR_565
    lv_draw_sw_rgb565_swap(color_map, lv_area_get_size(area));
    #endif

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}


void lv_port_run_task() {
    lv_tick_set_cb(lvgl_tick_cb);
    xTaskCreateStatic(lvgl_task_cb,
                        "lvgl",
                        LVGL_TASK_STACK,
                        NULL,
                        LVGL_TASK_PRIO,
                        lvgl_task_stack,
                        &lvgl_task_buffer);
}


void lv_port_init_display(esp_lcd_panel_handle_t panel, esp_lcd_panel_io_handle_t panel_io) {
    lv_display_t* display = lv_display_create(LVGL_DISPLAY_WIDTH, LVGL_DISPLAY_HEIGHT);

    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized

    // lv_color_t* buf = heap_caps_aligned_alloc(4, LVGL_DISPLAY_BUFFER_PIXELS * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    // lv_color_t* buf = heap_caps_aligned_alloc(4, LVGL_DISPLAY_BUFFER_PIXELS * sizeof(lv_color_t), MALLOC_CAP_DMA);
    
    #if defined (LVGL_USE_DOUBLE_BUFFER)
    lv_display_set_buffers(display, buf[0], buf[1], LVGL_DISPLAY_BUFFER_PIXELS* sizeof(lv_color_t), LV_DISP_RENDER_MODE_PARTIAL);
    #else
    lv_display_set_buffers(display, buf, NULL, LVGL_DISPLAY_BUFFER_PIXELS* sizeof(lv_color_t), LV_DISP_RENDER_MODE_PARTIAL);
    #endif

    lv_display_set_flush_cb(display, lvgl_flush_cb);
    lv_display_set_user_data(display, panel);

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    esp_lcd_panel_io_register_event_callbacks(panel_io, &cbs, display);

}
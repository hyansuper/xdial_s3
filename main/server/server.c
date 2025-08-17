#include "server.h"
#include "server_util.h"

static const char* const TAG = "server";
static httpd_handle_t server_hd;

extern const char index_start[] asm("_binary_index_html_start");
extern const char index_end[] asm("_binary_index_html_end");
static const server_rsc_t rsc_index = {.path="/",.start=index_start,.end=index_end, .type=TYPE_HTML,};


esp_err_t server_start() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.task_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT; // 用外部存储
    config.task_priority = 2; // 设置服务器只提供设置网页，优先级不同太高
    // config.lru_purge_enable = true;
    // config.max_uri_handlers = 10;
    ESP_RETURN_ON_ERROR(httpd_start(&server_hd, &config), TAG, "start server fail");
    
    serve_rsc(server_hd, &rsc_util);
    serve_rsc(server_hd, &rsc_index);
    return ESP_OK;
}

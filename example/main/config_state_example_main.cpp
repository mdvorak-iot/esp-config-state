#include "app_config.h"
#include <esp_log.h>

static const char TAG[] = "example";

static app_config config;

extern "C" void app_main()
{
    //app_config::STATE->get(config);

    // Setup complete
    ESP_LOGI(TAG, "started");
}

#include "config_state.h"
#include "config_state_gpio.h"
#include <esp_log.h>

static const char TAG[] = "example";

extern "C" void app_main()
{
    // Setup complete
    ESP_LOGI(TAG, "started");
}

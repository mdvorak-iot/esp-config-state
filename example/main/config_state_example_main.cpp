#include "config_state.h"
#include "config_state_gpio.h"
#include <esp_log.h>

static const char TAG[] = "example";

// TODO move to own file
struct example_config
{
    int num = 0;
};

static config_state_set<example_config> example_config_state;

extern "C" void app_main()
{
    example_config_state
        .add(new config_state_field<example_config, int>("/num", &example_config::num));

    // Setup complete
    ESP_LOGI(TAG, "started");
}

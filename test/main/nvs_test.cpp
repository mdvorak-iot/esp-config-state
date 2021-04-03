#include "app_config.h"
#include <unity.h>

static const std::unique_ptr<config_state<app_config>> APP_CONFIG_STATE = app_config::state();

extern "C" void include_nvs_test()
{
    // Empty function to force linking of otherwise unused file
}

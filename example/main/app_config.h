#pragma once

#include "config_state.h"
#include "config_state_gpio.h"

struct app_config
{
    int num = 0;
    gpio_num_t pin = GPIO_NUM_NC;
    std::string str;

    // TODO list, other types

    static const config_state<app_config> *const STATE;
};

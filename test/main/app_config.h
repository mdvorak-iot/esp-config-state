#pragma once

#include "config_state.h"
#include "config_state_gpio.h"

struct app_config_obj
{
    std::vector<uint32_t> ids;

    static const config_state<app_config_obj> *const STATE;
};

struct app_config
{
    int num = 0;
    gpio_num_t pin = GPIO_NUM_NC;
    std::string str;

    std::vector<int> num_list;
    std::vector<std::string> str_list;
    std::vector<app_config_obj> obj_list;

    static const config_state<app_config> *const STATE;
};

#pragma once

#include "config_state.h"
#include "config_state_gpio.h"

struct app_config_obj
{
    std::vector<uint32_t> ids;

    static std::unique_ptr<config_state<app_config_obj>> state()
    {
        auto ptr = &(*new config_state_set<app_config_obj>())
                        .add_value_list(&app_config_obj::ids, "/ids");

        return std::unique_ptr<config_state<app_config_obj>>(ptr);
    }
};

struct app_config
{
    int8_t num_i8 = 0;
    uint8_t num_u8 = 0;
    int16_t num_i16 = 0;
    uint16_t num_u16 = 0;
    int32_t num_i32 = 0;
    uint32_t num_u32 = 0;
    int num_int = 0;
    float num_float = 0;
    double num_double = 0;
    gpio_num_t pin = GPIO_NUM_NC;
    std::string str;

    std::vector<int> num_list;
    std::vector<std::string> str_list;
    std::vector<app_config_obj> obj_list;

    static std::unique_ptr<config_state<app_config>> state()
    {
        auto ptr = &(*new config_state_set<app_config>())
                        .add_field(&app_config::num_i8, "/numI8")
                        .add_field(&app_config::num_u8, "/numU8")
                        .add_field(&app_config::num_i16, "/numI16")
                        .add_field(&app_config::num_u16, "/numU16")
                        .add_field(&app_config::num_i32, "/numI32")
                        .add_field(&app_config::num_u32, "/numU32")
                        .add_field(&app_config::num_int, "/numInt", nullptr, config_state_disable_persistence)
                        .add_field(&app_config::num_float, "/numFloat", "/float")
                        .add_field(&app_config::num_double, "/numDouble")
                        .add_field(&app_config::pin, "/pin")
                        .add_field(&app_config::str, "/str")
                        .add_value_list(&app_config::num_list, "/numList")
                        .add_value_list(&app_config::str_list, "/strList")
                        .add_list(&app_config::obj_list, "/objList", "/ol", app_config_obj::state());

        return std::unique_ptr<config_state<app_config>>(ptr);
    }
};

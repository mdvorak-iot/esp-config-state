#pragma once

#include "config_state.h"
#include <hal/gpio_types.h>

template<>
bool config_state_helper<gpio_num_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, gpio_num_t &value);

template<>
esp_err_t config_state_helper<gpio_num_t>::load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, gpio_num_t &value);

template<>
esp_err_t config_state_helper<gpio_num_t>::store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const gpio_num_t &value);

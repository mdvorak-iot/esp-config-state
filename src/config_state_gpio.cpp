#include "config_state_gpio.h"

template<>
bool config_state_helper<gpio_num_t>::get(const rapidjson::Pointer &ptr, const rapidjson::Value &root, gpio_num_t &value)
{
    int num = value;
    bool changed = config_state_helper<int>::get(ptr, root, num);
    // TODO validate pin?
    value = static_cast<gpio_num_t>(num);
    return changed;
}

template<>
esp_err_t config_state_helper<gpio_num_t>::load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, gpio_num_t &value)
{
    int num = value;
    esp_err_t err = config_state_helper<int>::load(key, handle, prefix, num);
    if (err == ESP_OK)
    {
        value = static_cast<gpio_num_t>(num);
    }
    return err;
}

template<>
esp_err_t config_state_helper<gpio_num_t>::store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const gpio_num_t &value)
{
    return config_state_helper<int>::store(key, handle, prefix, static_cast<int>(value));
}

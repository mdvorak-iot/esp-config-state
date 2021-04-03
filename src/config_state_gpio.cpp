#include "config_state_gpio.h"

static inline bool is_valid_gpio(int gpio_num)
{
    return gpio_num == GPIO_NUM_NC || (gpio_num >= 0 && GPIO_IS_VALID_GPIO(gpio_num));
}

template<>
bool config_state_helper<gpio_num_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, gpio_num_t &value)
{
    int num = value;
    bool changed = config_state_helper<int>::read(ptr, root, num);
    if (changed && is_valid_gpio(num))
    {
        value = static_cast<gpio_num_t>(num);
        return true;
    }

    return false;
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

#pragma once

#include <nvs_handle.hpp>
#include <rapidjson/pointer.h>
#include <string>

__attribute__((format(printf, 1, 2))) void config_state_logw(const char *format, ...);

std::string config_state_nvs_key(const std::string &s);
const char *config_state_nvs_key(const char *s);

template<typename T>
struct config_state_helper
{
    /**
     * Gets value from given JSON object root and stores it to this instance.
     * Ignores invalid value type.
     *
     * @param root JSON root object
     * @param ptr JSON pointer
     * @param value Value reference
     * @return true if value has changed, false otherwise
     */
    static bool read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, T &value)
    {
        // Find object
        const rapidjson::Value *obj = ptr.Get(root);
        // Check its type
        if (obj && obj->Is<T>())
        {
            // Get new value
            T new_value = obj->Get<T>();
            if (new_value != value)
            {
                // If it is different, update
                value = new_value;
                return true;
            }
        }
        return false;
    }

    static void write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const T &value)
    {
        ptr.Set<T>(root, value, allocator);
    }

    static esp_err_t load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, T &value)
    {
        const std::string full_key = config_state_nvs_key(prefix && prefix[0] != '\0' ? prefix + key : key);
        esp_err_t err = handle.get_item<T>(full_key.c_str(), value);
        if (err != ESP_OK)
        {
            config_state_logw("failed to get_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }

    static esp_err_t store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const T &value)
    {
        const std::string full_key = config_state_nvs_key(prefix && prefix[0] != '\0' ? prefix + key : key);
        esp_err_t err = handle.set_item<T>(full_key.c_str(), value);
        if (err != ESP_OK)
        {
            config_state_logw("failed to set_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }
};

template<>
bool config_state_helper<std::string>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, std::string &value);

template<>
void config_state_helper<std::string>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const std::string &value);

template<>
bool config_state_helper<uint8_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, uint8_t &value);

template<>
void config_state_helper<uint8_t>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const uint8_t &value);

template<>
bool config_state_helper<int8_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, int8_t &value);

template<>
void config_state_helper<int8_t>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const int8_t &value);

template<>
bool config_state_helper<uint16_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, uint16_t &value);

template<>
void config_state_helper<uint16_t>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const uint16_t &value);

template<>
bool config_state_helper<int16_t>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, int16_t &value);

template<>
void config_state_helper<int16_t>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const int16_t &value);

template<>
esp_err_t config_state_helper<std::string>::load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, std::string &value);

template<>
esp_err_t config_state_helper<std::string>::store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const std::string &value);

template<>
esp_err_t config_state_helper<float>::load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, float &value);

template<>
esp_err_t config_state_helper<float>::store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const float &value);

template<>
esp_err_t config_state_helper<double>::load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, double &value);

template<>
esp_err_t config_state_helper<double>::store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const double &value);

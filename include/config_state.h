#pragma once

#include <cstring>
#include <esp_log.h>
#include <functional>
#include <nvs_handle.hpp>
#include <rapidjson/pointer.h>
#include <string>
#include <vector>

// TODO move to cpp, rename
inline static std::string nvs_key(const std::string &s)
{
    return !s.empty() && s[0] == '/' ? s.substr(1, std::string::npos) : s; // Skip leading '/' char
}

static const char *nvs_key(const char *s)
{
    assert(s);
    return *s == '/' ? s + 1 : s; // Skip leading '/' char
}

template<typename S>
struct config_state
{
    virtual ~config_state() = default;

    /**
     * Gets value from given JSON object root and stores it to this instance.
     * Ignores invalid value type.
     *
     * @param root JSON root object
     * @return true if value has changed, false otherwise
     */
    virtual bool get(const rapidjson::Value &root, S &inst) const = 0;
    virtual void set(rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const S &inst) const = 0;

    // TODO reorder to have prefix last with default value
    virtual esp_err_t load(nvs::NVSHandle &handle, const char *prefix, S &inst) const = 0;
    virtual esp_err_t store(nvs::NVSHandle &handle, const char *prefix, const S &inst) const = 0;
};

template<typename S>
struct config_state_set : config_state<S>
{
    config_state_set() = default;
    config_state_set(const config_state_set &) = delete;

    ~config_state_set() override
    {
        for (auto state : states_)
        {
            delete state;
        }
    }

    config_state_set &add(const config_state<S> *state)
    {
        assert(state);
        states_.push_back(state);
        return *this;
    }

    bool get(const rapidjson::Value &root, S &inst) const final
    {
        bool changed = false;
        for (auto state : states_)
        {
            changed |= state->get(root, inst);
        }
        return changed;
    }

    void set(rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const S &inst) const final
    {
        for (auto state : states_)
        {
            state->set(root, allocator, inst);
        }
    }

    esp_err_t load(nvs::NVSHandle &handle, const char *prefix, S &inst) const final
    {
        esp_err_t last_err = ESP_OK;
        for (auto state : states_)
        {
            esp_err_t err = state->load(handle, prefix, inst);
            if (err != ESP_OK && (err != ESP_ERR_NVS_NOT_FOUND || last_err == ESP_OK)) // Don't overwrite more important error with NOT_FOUND
            {
                last_err = err;
            }
        }
        return last_err;
    }

    esp_err_t store(nvs::NVSHandle &handle, const char *prefix, const S &inst) const final
    {
        esp_err_t last_err = ESP_OK;
        for (auto state : states_)
        {
            esp_err_t err = state->store(handle, prefix, inst);
            if (err != ESP_OK)
            {
                last_err = err;
            }
        }
        return last_err;
    }

 private:
    std::vector<const config_state<S> *> states_;
};

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
    static bool get(const rapidjson::Pointer &ptr, const rapidjson::Value &root, T &value)
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

    static void set(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const T &value)
    {
        ptr.Set<T>(root, value, allocator);
    }

    static esp_err_t load(const std::string &key, nvs::NVSHandle &handle, const char *prefix, T &value)
    {
        const std::string full_key = nvs_key(prefix && prefix[0] != '\0' ? prefix + key : key);
        esp_err_t err = handle.get_item<T>(full_key.c_str(), value);
        if (err != ESP_OK)
        {
            ESP_LOGW("config_state", "failed to get_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }

    static esp_err_t store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const T &value)
    {
        const std::string full_key = nvs_key(prefix && prefix[0] != '\0' ? prefix + key : key);
        esp_err_t err = handle.set_item<T>(full_key.c_str(), value);
        if (err != ESP_OK)
        {
            ESP_LOGW("config_state", "failed to set_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }
};

template<>
bool config_state_helper<std::string>::get(const rapidjson::Pointer &ptr, const rapidjson::Value &root, std::string &value);

template<>
void config_state_helper<std::string>::set(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const std::string &value);

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

template<typename S, typename T>
struct config_state_field : config_state<S>
{
    const rapidjson::Pointer ptr;
    const std::string key;
    T S::*const field;

    config_state_field(const char *json_ptr, T S::*field)
        : ptr(json_ptr),
          key(json_ptr),
          field(field)
    {
        assert(field);
    }

    bool get(const rapidjson::Value &root, S &inst) const final
    {
        return config_state_helper<T>::get(ptr, root, inst.*field);
    }

    void set(rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const S &inst) const final
    {
        config_state_helper<T>::set(ptr, root, allocator, inst.*field);
    }

    esp_err_t load(nvs::NVSHandle &handle, const char *prefix, S &inst) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst.*field);
    }

    esp_err_t store(nvs::NVSHandle &handle, const char *prefix, const S &inst) const final
    {
        return config_state_helper<T>::store(key, handle, prefix, inst.*field);
    }
};

template<typename T>
struct config_state_value : config_state<T>
{
    const rapidjson::Pointer ptr;
    const std::string key;

    explicit config_state_value(const char *json_ptr)
        : ptr(json_ptr),
          key(json_ptr)
    {
    }

    bool get(const rapidjson::Value &root, T &inst) const final
    {
        return config_state_helper<T>::get(ptr, root, inst);
    }

    void set(rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const T &inst) const final
    {
        config_state_helper<T>::set(ptr, root, allocator, inst);
    }

    esp_err_t load(nvs::NVSHandle &handle, const char *prefix, T &inst) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst);
    }

    esp_err_t store(nvs::NVSHandle &handle, const char *prefix, const T &inst) const final
    {
        return config_state_helper<T>::store(key, handle, prefix, inst);
    }
};

template<typename S, typename T>
struct config_state_list : config_state<S>
{
    const rapidjson::Pointer ptr;
    const std::string key;
    std::vector<T> S::*const field;
    const std::unique_ptr<const config_state<T>> element;

    config_state_list(const char *json_ptr, std::vector<T> S::*field, const config_state<T> *element)
        : ptr(json_ptr),
          key(json_ptr),
          field(field),
          element(element)
    {
        assert(field);
        assert(element);
    }

    bool get(const rapidjson::Value &root, S &inst) const final
    {
        const rapidjson::Value *list = ptr.Get(root);
        if (!list || !list->IsArray())
        {
            return false;
        }

        auto &items = inst.*field;

        // Resize
        auto array = list->GetArray();
        size_t length = array.Size();

        items.resize(length);

        // Get each
        bool changed = false;
        for (size_t i = 0; i < length; i++)
        {
            changed |= element->get(array[i], items[i]);
        }
        return changed;
    }

    void set(rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const S &inst) const final
    {
        auto &array = ptr.Create(root, allocator);
        if (!array.IsArray())
        {
            array.SetArray();
        }

        auto &items = inst.*field;
        size_t len = items.size();

        // Resize array
        array.Reserve(len, allocator);

        while (array.Size() < len)
        {
            array.PushBack(rapidjson::Value(), allocator);
        }
        while (array.Size() > len)
        {
            array.PopBack();
        }

        // Set each
        for (size_t i = 0; i < len; i++)
        {
            element->set(array[i], allocator, items[i]);
        }
    }

    esp_err_t load(nvs::NVSHandle &handle, const char *prefix, S &inst) const final
    {
        char item_prefix[16] = {};
        if (!prefix) prefix = "";

        auto &items = inst.*field;

        // Read length
        std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/len", prefix, key.c_str());
        uint16_t length = 0;
        handle.get_item(nvs_key(item_prefix), length); // Ignore error

        // Resize
        items.resize(length);

        // Read items
        esp_err_t last_err = ESP_OK;
        for (size_t i = 0; i < items.size(); i++)
        {
            std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/%uz", prefix, key.c_str(), i);
            esp_err_t err = element->load(handle, nvs_key(item_prefix), items[i]);
            if (err != ESP_OK && (err != ESP_ERR_NVS_NOT_FOUND || last_err == ESP_OK)) // Don't overwrite more important error with NOT_FOUND
            {
                last_err = err;
            }
        }
        return last_err;
    }

    esp_err_t store(nvs::NVSHandle &handle, const char *prefix, const S &inst) const final
    {
        char item_prefix[16] = {};
        if (!prefix) prefix = "";

        auto &items = inst.*field;

        // Store length
        std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/len", prefix, key.c_str());
        handle.set_item(nvs_key(item_prefix), static_cast<uint16_t>(items.size())); // No need to store all 32 bytes, that would never fit in memory

        // Store items
        esp_err_t last_err = ESP_OK;
        for (size_t i = 0; i < items.size(); i++)
        {
            std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/%uz", prefix, key.c_str(), i);
            esp_err_t err = element->store(handle, nvs_key(item_prefix), items[i]);
            if (err != ESP_OK)
            {
                last_err = err;
            }
        }
        return last_err;
    }
};

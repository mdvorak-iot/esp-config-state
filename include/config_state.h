#pragma once

#include <cstring>
#include <esp_log.h>
#include <functional>
#include <nvs_handle.hpp>
#include <rapidjson/pointer.h>
#include <string>
#include <vector>

// TODO move to namespace

std::string config_state_nvs_key(const std::string &s);
const char *config_state_nvs_key(const char *s);

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
    virtual bool read(S &inst, const rapidjson::Value &root) const = 0;
    virtual void write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const = 0;

    virtual esp_err_t load(S &inst, nvs::NVSHandle &handle, const char *prefix) const = 0;
    virtual esp_err_t store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const = 0;
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
            ESP_LOGW("config_state", "failed to get_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }

    static esp_err_t store(const std::string &key, nvs::NVSHandle &handle, const char *prefix, const T &value)
    {
        const std::string full_key = config_state_nvs_key(prefix && prefix[0] != '\0' ? prefix + key : key);
        esp_err_t err = handle.set_item<T>(full_key.c_str(), value);
        if (err != ESP_OK)
        {
            ESP_LOGW("config_state", "failed to set_item %s: %d %s", full_key.c_str(), err, esp_err_to_name(err));
        }
        return err;
    }
};

template<>
bool config_state_helper<std::string>::read(const rapidjson::Pointer &ptr, const rapidjson::Value &root, std::string &value);

template<>
void config_state_helper<std::string>::write(const rapidjson::Pointer &ptr, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator, const std::string &value);

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

    bool read(S &inst, const rapidjson::Value &root) const final
    {
        return config_state_helper<T>::read(ptr, root, inst.*field);
    }

    void write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        config_state_helper<T>::write(ptr, root, allocator, inst.*field);
    }

    esp_err_t load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst.*field);
    }

    esp_err_t store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

    config_state_value()
        : config_state_value("")
    {
    }

    bool read(T &inst, const rapidjson::Value &root) const final
    {
        return config_state_helper<T>::read(ptr, root, inst);
    }

    void write(const T &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        config_state_helper<T>::write(ptr, root, allocator, inst);
    }

    esp_err_t load(T &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst);
    }

    esp_err_t store(const T &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

    bool read(S &inst, const rapidjson::Value &root) const final
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
            changed |= element->read(items[i], array[i]);
        }
        return changed;
    }

    void write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
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
            element->write(items[i], array[i], allocator);
        }
    }

    esp_err_t load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        char item_prefix[16] = {};
        if (!prefix) prefix = "";

        auto &items = inst.*field;

        // Read length
        std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/len", prefix, key.c_str());
        uint16_t length = 0;
        handle.get_item(config_state_nvs_key(item_prefix), length); // Ignore error

        // Resize
        items.resize(length);

        // Read items
        esp_err_t last_err = ESP_OK;
        for (size_t i = 0; i < items.size(); i++)
        {
            std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/%uz", prefix, key.c_str(), i);
            esp_err_t err = element->load(items[i], handle, config_state_nvs_key(item_prefix));
            if (err != ESP_OK && (err != ESP_ERR_NVS_NOT_FOUND || last_err == ESP_OK)) // Don't overwrite more important error with NOT_FOUND
            {
                last_err = err;
            }
        }
        return last_err;
    }

    esp_err_t store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        char item_prefix[16] = {};
        if (!prefix) prefix = "";

        auto &items = inst.*field;

        // Store length
        std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/len", prefix, key.c_str());
        handle.set_item(config_state_nvs_key(item_prefix), static_cast<uint16_t>(items.size())); // No need to store all 32 bytes, that would never fit in memory

        // Store items
        esp_err_t last_err = ESP_OK;
        for (size_t i = 0; i < items.size(); i++)
        {
            std::snprintf(item_prefix, sizeof(item_prefix) - 1, "%s%s/%uz", prefix, key.c_str(), i);
            esp_err_t err = element->store(items[i], handle, config_state_nvs_key(item_prefix));
            if (err != ESP_OK)
            {
                last_err = err;
            }
        }
        return last_err;
    }
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

    template<typename T>
    config_state_set &add_field(const char *json_ptr, T S::*field)
    {
        return add(new config_state_field<S, T>(json_ptr, field));
    }

    template<typename T>
    config_state_set &add_list(const char *json_ptr, std::vector<T> S::*field, const config_state<T> *element)
    {
        return add(new config_state_list<S, T>(json_ptr, field, element));
    }

    template<typename T>
    config_state_set &add_value_list(const char *json_ptr, std::vector<T> S::*field)
    {
        return add(new config_state_list<S, T>(json_ptr, field, new config_state_value<T>));
    }

    bool read(S &inst, const rapidjson::Value &root) const final
    {
        bool changed = false;
        for (auto state : states_)
        {
            changed |= state->read(inst, root);
        }
        return changed;
    }

    void write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        for (auto state : states_)
        {
            state->write(inst, root, allocator);
        }
    }

    esp_err_t load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        esp_err_t last_err = ESP_OK;
        for (auto state : states_)
        {
            esp_err_t err = state->load(inst, handle, prefix);
            if (err != ESP_OK && (err != ESP_ERR_NVS_NOT_FOUND || last_err == ESP_OK)) // Don't overwrite more important error with NOT_FOUND
            {
                last_err = err;
            }
        }
        return last_err;
    }

    esp_err_t store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        esp_err_t last_err = ESP_OK;
        for (auto state : states_)
        {
            esp_err_t err = state->store(inst, handle, prefix);
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

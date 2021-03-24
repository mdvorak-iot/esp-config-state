#pragma once

#include "config_state_helper.h"
#include <nvs_handle.hpp>
#include <rapidjson/pointer.h>
#include <string>
#include <vector>

// TODO move to namespace

enum config_state_flags
{
    config_state_no_flags = 0,
    config_state_disable_read = 0x01,
    config_state_disable_write = 0x02,
    config_state_disable_serialization = config_state_disable_read | config_state_disable_write,
    config_state_disable_load = 0x10,
    config_state_disable_store = 0x20,
    config_state_disable_persistence = config_state_disable_load | config_state_disable_store,
};

template<typename S>
struct config_state
{
    const config_state_flags flags;

    explicit config_state(config_state_flags flags)
        : flags(flags)
    {
    }

    virtual ~config_state() = default;

    /**
     * Gets value from given JSON object root and stores it to this instance.
     * Ignores invalid value type.
     *
     * @param root JSON root object
     * @return true if value has changed, false otherwise
     */
    bool read(S &inst, const rapidjson::Value &root) const
    {
        if ((flags & config_state_disable_read) == 0)
        {
            return do_read(inst, root);
        }
        return false;
    }

    void write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const
    {
        if ((flags & config_state_disable_write) == 0)
        {
            do_write(inst, root, allocator);
        }
    }

    esp_err_t load(S &inst, nvs::NVSHandle &handle, const char *prefix) const
    {
        if ((flags & config_state_disable_load) == 0)
        {
            return do_load(inst, handle, prefix);
        }
        return ESP_OK;
    }
    esp_err_t store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const
    {
        if ((flags & config_state_disable_store) == 0)
        {
            return do_store(inst, handle, prefix);
        }
        return ESP_OK;
    }

    virtual bool do_read(S &inst, const rapidjson::Value &root) const = 0;
    virtual void do_write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const = 0;

    virtual esp_err_t do_load(S &inst, nvs::NVSHandle &handle, const char *prefix) const = 0;
    virtual esp_err_t do_store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const = 0;
};

template<typename S, typename T>
struct config_state_field : config_state<S>
{
    const rapidjson::Pointer ptr;
    const std::string key;
    T S::*const field;

    explicit config_state_field(T S::*field, const char *json_ptr, const char *nvs_key = nullptr, config_state_flags flags = config_state_no_flags)
        : config_state<S>(flags),
          ptr(json_ptr),
          key(nvs_key ? nvs_key : json_ptr),
          field(field)
    {
        assert(field);
    }

    bool do_read(S &inst, const rapidjson::Value &root) const final
    {
        return config_state_helper<T>::read(ptr, root, inst.*field);
    }

    void do_write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        config_state_helper<T>::write(ptr, root, allocator, inst.*field);
    }

    esp_err_t do_load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst.*field);
    }

    esp_err_t do_store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        return config_state_helper<T>::store(key, handle, prefix, inst.*field);
    }
};

template<typename T>
struct config_state_value : config_state<T>
{
    const rapidjson::Pointer ptr;
    const std::string key;

    explicit config_state_value(config_state_flags flags = config_state_no_flags)
        : config_state_value("", "", flags)
    {
    }

    explicit config_state_value(const char *json_ptr = "", const char *nvs_key = nullptr, config_state_flags flags = config_state_no_flags)
        : config_state<T>(flags),
          ptr(json_ptr),
          key(nvs_key ? nvs_key : json_ptr)
    {
    }

    bool do_read(T &inst, const rapidjson::Value &root) const final
    {
        return config_state_helper<T>::read(ptr, root, inst);
    }

    void do_write(const T &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        config_state_helper<T>::write(ptr, root, allocator, inst);
    }

    esp_err_t do_load(T &inst, nvs::NVSHandle &handle, const char *prefix) const final
    {
        return config_state_helper<T>::load(key, handle, prefix, inst);
    }

    esp_err_t do_store(const T &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

    config_state_list(std::vector<T> S::*field, const char *json_ptr, const config_state<T> *element, config_state_flags flags = config_state_no_flags)
        : config_state_list(field, json_ptr, nullptr, element, flags)
    {
    }

    config_state_list(std::vector<T> S::*field, const char *json_ptr, const char *nvs_key, const config_state<T> *element, config_state_flags flags = config_state_no_flags)
        : config_state<S>(flags),
          ptr(json_ptr),
          key(nvs_key ? nvs_key : json_ptr),
          field(field),
          element(element)
    {
        assert(field);
        assert(element);
    }

    bool do_read(S &inst, const rapidjson::Value &root) const final
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

    void do_write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
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

    esp_err_t do_load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

    esp_err_t do_store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
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
    explicit config_state_set(config_state_flags flags = config_state_no_flags)
        : config_state<S>(flags)
    {
    }

    // disable copy
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
    config_state_set &add_field(T S::*field, const char *json_ptr, const char *nvs_key = nullptr, config_state_flags flags = config_state_no_flags)
    {
        return add(new config_state_field<S, T>(field, json_ptr, nvs_key, flags));
    }

    template<typename T>
    config_state_set &add_list(std::vector<T> S::*field, const char *json_ptr, const config_state<T> *element, config_state_flags flags = config_state_no_flags)
    {
        return add(new config_state_list<S, T>(field, json_ptr, element, flags));
    }

    template<typename T>
    config_state_set &add_list(std::vector<T> S::*field, const char *json_ptr, const char *nvs_key, const config_state<T> *element, config_state_flags flags = config_state_no_flags)
    {
        return add(new config_state_list<S, T>(field, json_ptr, nvs_key, element, flags));
    }

    template<typename T>
    config_state_set &add_value_list(std::vector<T> S::*field, const char *json_ptr, const char *nvs_key = nullptr, config_state_flags flags = config_state_no_flags)
    {
        return add(new config_state_list<S, T>(field, json_ptr, nvs_key, new config_state_value<T>(flags)));
    }

    bool do_read(S &inst, const rapidjson::Value &root) const final
    {
        bool changed = false;
        for (auto state : states_)
        {
            changed |= state->read(inst, root);
        }
        return changed;
    }

    void do_write(const S &inst, rapidjson::Value &root, rapidjson::Value::AllocatorType &allocator) const final
    {
        for (auto state : states_)
        {
            state->write(inst, root, allocator);
        }
    }

    esp_err_t do_load(S &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

    esp_err_t do_store(const S &inst, nvs::NVSHandle &handle, const char *prefix) const final
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

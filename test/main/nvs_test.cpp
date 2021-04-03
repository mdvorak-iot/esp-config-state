#include "app_config.h"
#include <nvs_flash.h>
#include <unity.h>

static const std::unique_ptr<config_state<app_config>> APP_CONFIG_STATE = app_config::state();

extern "C" void include_nvs_test()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

TEST_CASE("load empty config", "[nvs][load]")
{
    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("unit_test", NVS_READWRITE, &err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(handle.get());

    // Test
    app_config config = {};
    config.num_int = 21;
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_NOT_FOUND, APP_CONFIG_STATE->load(config, handle));

    // Verify
    TEST_ASSERT_EQUAL(21, config.num_int);
}

TEST_CASE("load full config", "[nvs][load]")
{
    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("unit_test", NVS_READWRITE, &err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(handle.get());

    handle->set_item<int8_t>("numI8", -1);
    handle->set_item<uint8_t>("numU8", 2);
    handle->set_item<int16_t>("numI16", -3);
    handle->set_item<uint16_t>("numU16", 4);
    handle->set_item<int32_t>("numI32", -5);
    handle->set_item<uint32_t>("numU32", 6);
    handle->set_item<int>("numInt", -7);
    const float f = 8.5f;
    handle->set_item<uint32_t>("float", *(uint32_t *)&f); // custom key
    const double d = 9.5;
    handle->set_item<uint64_t>("numDouble", *(uint64_t *)&d);
    handle->set_item<bool>("boolean", true);
    handle->set_item<int>("pin", 22);
    handle->set_string("str", "foobar1234");
    handle->set_item<uint16_t>("numList/len", 2);
    handle->set_item<int>("numList/0", 10);
    handle->set_item<int>("numList/1", 11);
    handle->set_item<int>("numList/2", 12);
    handle->set_item<uint16_t>("strList/len", 1);
    handle->set_string("strList/0", "goo");
    handle->set_item<uint16_t>("ol/len", 1); // custom key
    handle->set_item<uint16_t>("ol/0/ids/len", 2);
    handle->set_item<uint32_t>("ol/0/ids/0", 13);
    handle->set_item<uint32_t>("ol/0/ids/1", 14);

    // Test
    app_config config = {};
    TEST_ASSERT_EQUAL(ESP_OK, APP_CONFIG_STATE->load(config, handle));

    // Verify
    TEST_ASSERT_EQUAL(-1, config.num_i8);
    TEST_ASSERT_EQUAL(2, config.num_u8);
    TEST_ASSERT_EQUAL(-3, config.num_i16);
    TEST_ASSERT_EQUAL(4, config.num_u16);
    TEST_ASSERT_EQUAL(-5, config.num_i32);
    TEST_ASSERT_EQUAL(6, config.num_u32);
    TEST_ASSERT_EQUAL(-7, config.num_int);
    TEST_ASSERT_EQUAL(8.5f, config.num_float);
    TEST_ASSERT_EQUAL(9.5, config.num_double);
    TEST_ASSERT_EQUAL(true, config.boolean);
    TEST_ASSERT_EQUAL(22, config.pin);
    TEST_ASSERT_EQUAL(3, config.num_list.size());
    TEST_ASSERT_EQUAL(10, config.num_list[0]);
    TEST_ASSERT_EQUAL(11, config.num_list[1]);
    TEST_ASSERT_EQUAL(12, config.num_list[2]);
    TEST_ASSERT_EQUAL(1, config.str_list.size());
    TEST_ASSERT_EQUAL_STRING("goo", config.str_list[0].c_str());
    TEST_ASSERT_EQUAL(1, config.obj_list.size());
    TEST_ASSERT_EQUAL(2, config.obj_list[0].ids.size());
    TEST_ASSERT_EQUAL(13, config.obj_list[0].ids[0]);
    TEST_ASSERT_EQUAL(14, config.obj_list[0].ids[1]);
}

TEST_CASE("check store error", "[nvs][store]")
{
    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("unit_test", NVS_READWRITE, &err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(handle.get());

    // Custom state for this test
    // NOTE order matters for this test
    config_state_set<app_config> state;
    state.add_field(&app_config::num_u8, "/x");
    state.add_field(&app_config::num_int, "/int", "/abcdefg123456789"); // key too long
    state.add_field(&app_config::num_i8, "/a");
    state.add_field(&app_config::boolean, "/b");

    // Test
    app_config config = {};
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_KEY_TOO_LONG, state.store(config, handle));
}

// TODO test store

// TODO test flags

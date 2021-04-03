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

// TODO test load

// TODO test store

// TODO test flags

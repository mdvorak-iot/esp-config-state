#include "app_config.h"
#include <nvs_flash.h>
#include <unity.h>

static const char NVS_TEST_NAMESPACE[] = "unit_test";
static const std::unique_ptr<config_state<app_config>> APP_CONFIG_STATE = app_config::state();

extern "C" void test_nvs_cleanup()
{
    esp_err_t err = ESP_FAIL;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(NVS_TEST_NAMESPACE, NVS_READWRITE, &err);
    ESP_ERROR_CHECK(handle->erase_all());
    ESP_ERROR_CHECK(handle->commit());
}

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
    // Setup
    test_nvs_cleanup();

    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(NVS_TEST_NAMESPACE, NVS_READWRITE, &err);
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
    // Setup
    test_nvs_cleanup();

    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(NVS_TEST_NAMESPACE, NVS_READWRITE, &err);
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

TEST_CASE("store and load full config", "[nvs][store]")
{
    // Setup
    test_nvs_cleanup();

    esp_err_t err = ESP_OK;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(NVS_TEST_NAMESPACE, NVS_READWRITE, &err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(handle.get());

    // Data
    app_config expected = {};
    expected.num_i8 = -7;
    expected.num_u8 = 8;
    expected.num_i16 = -15;
    expected.num_u16 = 16;
    expected.num_i32 = -31;
    expected.num_u32 = 32;
    expected.num_int = -40;
    expected.num_float = 42.123456;
    expected.num_double = -43.123456;
    expected.boolean = true;
    expected.pin = GPIO_NUM_22;
    expected.str = "foobar";
    expected.num_list.push_back(4);
    expected.num_list.push_back(-8);
    expected.num_list.push_back(6);
    expected.str_list.emplace_back("x");
    expected.str_list.emplace_back("y");

    app_config_obj obj;
    obj.ids.push_back(55);
    obj.ids.push_back(88);
    expected.obj_list.push_back(obj);

    // Test
    TEST_ASSERT_EQUAL(ESP_OK, APP_CONFIG_STATE->store(expected, handle));
    TEST_ASSERT_EQUAL(ESP_OK, handle->commit());
    handle.reset(); // close handle

    handle = nvs::open_nvs_handle(NVS_TEST_NAMESPACE, NVS_READONLY, &err); // re-open readonly
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(handle.get());

    app_config loaded = {};
    TEST_ASSERT_EQUAL(ESP_OK, APP_CONFIG_STATE->load(loaded, handle));

    // Verify config
    TEST_ASSERT_EQUAL(expected.num_i8, loaded.num_i8);
    TEST_ASSERT_EQUAL(expected.num_u8, loaded.num_u8);
    TEST_ASSERT_EQUAL(expected.num_i16, loaded.num_i16);
    TEST_ASSERT_EQUAL(expected.num_u16, loaded.num_u16);
    TEST_ASSERT_EQUAL(expected.num_i32, loaded.num_i32);
    TEST_ASSERT_EQUAL(expected.num_u32, loaded.num_u32);
    TEST_ASSERT_EQUAL(0, loaded.num_int); // disabled persistence for this field
    TEST_ASSERT_EQUAL(expected.num_float, loaded.num_float);
    TEST_ASSERT_EQUAL(expected.num_double, loaded.num_double);
    TEST_ASSERT_EQUAL(expected.boolean, loaded.boolean);
    TEST_ASSERT_EQUAL(expected.pin, loaded.pin);
    TEST_ASSERT_EQUAL(expected.num_list.size(), loaded.num_list.size());
    TEST_ASSERT_EQUAL(expected.num_list[0], loaded.num_list[0]);
    TEST_ASSERT_EQUAL(expected.num_list[1], loaded.num_list[1]);
    TEST_ASSERT_EQUAL(expected.str_list.size(), loaded.str_list.size());
    TEST_ASSERT_EQUAL_STRING(expected.str_list[0].c_str(), loaded.str_list[0].c_str());
    TEST_ASSERT_EQUAL(expected.obj_list.size(), loaded.obj_list.size());
    TEST_ASSERT_EQUAL(expected.obj_list[0].ids.size(), loaded.obj_list[0].ids.size());
    TEST_ASSERT_EQUAL(expected.obj_list[0].ids[0], loaded.obj_list[0].ids[0]);
    TEST_ASSERT_EQUAL(expected.obj_list[0].ids[1], loaded.obj_list[0].ids[1]);

    // Verify nvs
    int8_t num_i8 = 0;
    TEST_ASSERT_EQUAL(ESP_OK, handle->get_item("numI8", num_i8));
    TEST_ASSERT_EQUAL(-7, num_i8);
    uint16_t ol_len = 0;
    TEST_ASSERT_EQUAL(ESP_OK, handle->get_item("ol/len", ol_len));
    TEST_ASSERT_EQUAL(1, ol_len);
    uint16_t ol_ids_len = 0;
    TEST_ASSERT_EQUAL(ESP_OK, handle->get_item("ol/0/ids/len", ol_ids_len));
    TEST_ASSERT_EQUAL(2, ol_ids_len);
    uint32_t id_0 = 0;
    TEST_ASSERT_EQUAL(ESP_OK, handle->get_item("ol/0/ids/0", id_0));
    TEST_ASSERT_EQUAL(55, id_0);
}

// TODO test flags

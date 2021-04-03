#include <esp_sleep.h>
#include <unity.h>

void include_json_test();
void include_nvs_test();
void test_nvs_cleanup();

void app_main()
{
    include_json_test();
    include_nvs_test();

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    // Cleanup after tests
    test_nvs_cleanup();

    // Power-save mode
    esp_deep_sleep_start();
}

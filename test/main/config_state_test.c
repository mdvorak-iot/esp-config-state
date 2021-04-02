#include <esp_sleep.h>
#include <unity.h>

void include_json_test();

void app_main()
{
    include_json_test();

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    // Power-save mode
    esp_deep_sleep_start();
}

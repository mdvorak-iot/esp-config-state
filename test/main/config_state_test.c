#include <unity.h>

void include_json_test();

void app_main()
{
    include_json_test();

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

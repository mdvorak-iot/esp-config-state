#include <unity.h>

void include_json_test();

void app_main()
{
    include_json_test(); // Empty function to force linking of otherwise unused file

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

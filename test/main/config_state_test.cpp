#include "app_config.h"
#include <unity.h>

static const char TAG[] = "config_state";

static app_config config;

TEST_CASE("read empty document", TAG)
{
    rapidjson::Document doc;
    TEST_ASSERT_FALSE(app_config::STATE->read(config, doc));
}

extern "C" void app_main()
{
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

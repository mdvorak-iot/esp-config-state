#include "app_config.h"
#include <unity.h>

static const std::unique_ptr<config_state<app_config>> APP_CONFIG_STATE = app_config::state();

extern "C" void include_json_test()
{
    // Empty function to force linking of otherwise unused file
}

TEST_CASE("read empty document", "[json][read]")
{
    app_config config = {};
    rapidjson::Document doc;
    doc.SetObject();

    config.num_int = 21;
    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));
    TEST_ASSERT_EQUAL(21, config.num_int);
}

TEST_CASE("read document", "[json][read]")
{
    app_config config = {};
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("numInt", 41, doc.GetAllocator());
    doc.AddMember("numFloat", 42.123456, doc.GetAllocator());
    doc.AddMember("numDouble", 43.123456, doc.GetAllocator());

    TEST_ASSERT_TRUE(APP_CONFIG_STATE->read(config, doc));
    TEST_ASSERT_EQUAL(41, config.num_int);
    TEST_ASSERT_EQUAL(42.123456, config.num_float);
    TEST_ASSERT_EQUAL(43.123456, config.num_double);

    // Repeated read should be without change
    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));
}

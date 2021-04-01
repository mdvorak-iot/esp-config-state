#include "app_config.h"
#include <unity.h>

extern "C" void include_json_test()
{
}

TEST_CASE("read empty document", "[json][read]")
{
    app_config config = {};
    rapidjson::Document doc;
    doc.SetObject();

    config.integer = 21;
    TEST_ASSERT_FALSE(app_config::STATE->read(config, doc));
    TEST_ASSERT_EQUAL(21, config.integer);
}

TEST_CASE("read document", "[json][read]")
{
    app_config config = {};
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("number", 42.123456, doc.GetAllocator());

    TEST_ASSERT_TRUE(app_config::STATE->read(config, doc));
    TEST_ASSERT_EQUAL(42.123456, config.number);

    // Repeated read should be without change
    TEST_ASSERT_FALSE(app_config::STATE->read(config, doc));
}

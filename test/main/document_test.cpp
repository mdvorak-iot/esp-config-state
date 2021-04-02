#include "app_config.h"
#include <iostream>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <unity.h>

static const std::unique_ptr<config_state<app_config>> APP_CONFIG_STATE = app_config::state();

extern "C" void include_json_test()
{
    // Empty function to force linking of otherwise unused file
}

static void print_document(const rapidjson::Document &doc)
{
    rapidjson::OStreamWrapper out(std::cout);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(out);
    doc.Accept(writer);
    std::cout << std::endl;
}

TEST_CASE("read empty document", "[json][read]")
{
    app_config config = {};
    rapidjson::Document doc;
    doc.SetObject();

    print_document(doc);

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
    doc.AddMember("pin", 22, doc.GetAllocator());
    doc.AddMember("str", "foobar", doc.GetAllocator());

    rapidjson::Value num_list;
    num_list.SetArray();
    num_list.PushBack(4, doc.GetAllocator());
    num_list.PushBack(8, doc.GetAllocator());
    num_list.PushBack(6, doc.GetAllocator());
    doc.AddMember("numList", num_list, doc.GetAllocator());

    rapidjson::Value obj_ids;
    obj_ids.SetArray();
    obj_ids.PushBack(55, doc.GetAllocator());
    obj_ids.PushBack(88, doc.GetAllocator());

    rapidjson::Value obj;
    obj.SetObject();
    obj.AddMember("ids", obj_ids, doc.GetAllocator());

    rapidjson::Value obj_list;
    obj_list.SetArray();
    obj_list.PushBack(obj, doc.GetAllocator());
    doc.AddMember("objList", obj_list, doc.GetAllocator());

    print_document(doc);

    TEST_ASSERT_TRUE(APP_CONFIG_STATE->read(config, doc));
    TEST_ASSERT_EQUAL(41, config.num_int);
    TEST_ASSERT_EQUAL(42.123456, config.num_float);
    TEST_ASSERT_EQUAL(43.123456, config.num_double);
    TEST_ASSERT_EQUAL(22, config.pin);
    TEST_ASSERT_EQUAL_STRING("foobar", config.str.c_str());
    TEST_ASSERT_EQUAL(3, config.num_list.size());
    TEST_ASSERT_EQUAL(4, config.num_list[0]);
    TEST_ASSERT_EQUAL(8, config.num_list[1]);
    TEST_ASSERT_EQUAL(6, config.num_list[2]);
    TEST_ASSERT_EQUAL(1, config.obj_list.size());
    auto &ids = config.obj_list[0].ids;
    TEST_ASSERT_EQUAL(2, ids.size());
    TEST_ASSERT_EQUAL(55, ids[0]);
    TEST_ASSERT_EQUAL(88, ids[1]);

    // Repeated read should be without change
    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));
}

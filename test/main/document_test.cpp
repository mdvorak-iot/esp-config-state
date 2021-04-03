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
    rapidjson::Document doc;
    doc.SetObject();

    print_document(doc);

    // Test
    app_config config = {};
    config.num_int = 21;
    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));

    // Verify
    TEST_ASSERT_EQUAL(21, config.num_int);
}

TEST_CASE("read document", "[json][read]")
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("numI8", 7, doc.GetAllocator());
    doc.AddMember("numU8", 8, doc.GetAllocator());
    doc.AddMember("numI16", 15, doc.GetAllocator());
    doc.AddMember("numU16", 16, doc.GetAllocator());
    doc.AddMember("numI32", 31, doc.GetAllocator());
    doc.AddMember("numU32", 32, doc.GetAllocator());
    doc.AddMember("numInt", 41, doc.GetAllocator());
    doc.AddMember("numFloat", 42.123456, doc.GetAllocator());
    doc.AddMember("numDouble", 43.123456, doc.GetAllocator());
    doc.AddMember("boolean", true, doc.GetAllocator());
    doc.AddMember("pin", 22, doc.GetAllocator());
    doc.AddMember("str", "foobar", doc.GetAllocator());

    rapidjson::Value num_list;
    num_list.SetArray();
    num_list.PushBack(4, doc.GetAllocator());
    num_list.PushBack(8, doc.GetAllocator());
    num_list.PushBack(6, doc.GetAllocator());
    doc.AddMember("numList", num_list, doc.GetAllocator());

    rapidjson::Value str_list;
    str_list.SetArray();
    str_list.PushBack("a", doc.GetAllocator());
    str_list.PushBack("b", doc.GetAllocator());
    doc.AddMember("strList", str_list, doc.GetAllocator());

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

    // Test
    app_config config = {};
    TEST_ASSERT_TRUE(APP_CONFIG_STATE->read(config, doc));

    // Verify
    TEST_ASSERT_EQUAL(7, config.num_i8);
    TEST_ASSERT_EQUAL(8, config.num_u8);
    TEST_ASSERT_EQUAL(15, config.num_i16);
    TEST_ASSERT_EQUAL(16, config.num_u16);
    TEST_ASSERT_EQUAL(31, config.num_i32);
    TEST_ASSERT_EQUAL(32, config.num_u32);
    TEST_ASSERT_EQUAL(41, config.num_int);
    TEST_ASSERT_EQUAL(42.123456, config.num_float);
    TEST_ASSERT_EQUAL(43.123456, config.num_double);
    TEST_ASSERT_EQUAL(true, config.boolean);
    TEST_ASSERT_EQUAL(22, config.pin);
    TEST_ASSERT_EQUAL_STRING("foobar", config.str.c_str());
    TEST_ASSERT_EQUAL(3, config.num_list.size());
    TEST_ASSERT_EQUAL(4, config.num_list[0]);
    TEST_ASSERT_EQUAL(8, config.num_list[1]);
    TEST_ASSERT_EQUAL(6, config.num_list[2]);
    TEST_ASSERT_EQUAL(2, config.str_list.size());
    TEST_ASSERT_EQUAL_STRING("a", config.str_list[0].c_str());
    TEST_ASSERT_EQUAL_STRING("b", config.str_list[1].c_str());
    TEST_ASSERT_EQUAL(1, config.obj_list.size());
    auto &ids = config.obj_list[0].ids;
    TEST_ASSERT_EQUAL(2, ids.size());
    TEST_ASSERT_EQUAL(55, ids[0]);
    TEST_ASSERT_EQUAL(88, ids[1]);

    // Repeated read should be without change
    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));
}

TEST_CASE("read small numbers out of range (max)", "[json][read]")
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("numI8", (int)INT8_MAX + 1, doc.GetAllocator());
    doc.AddMember("numU8", (int)UINT8_MAX + 1, doc.GetAllocator());
    doc.AddMember("numI16", (int)INT16_MAX + 1, doc.GetAllocator());
    doc.AddMember("numU16", (int)UINT16_MAX + 1, doc.GetAllocator());

    print_document(doc);

    // Test
    app_config config = {};
    config.num_i8 = 7;
    config.num_u8 = 8;
    config.num_i16 = 15;
    config.num_u16 = 16;

    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));

    // Verify
    TEST_ASSERT_EQUAL(7, config.num_i8);
    TEST_ASSERT_EQUAL(8, config.num_u8);
    TEST_ASSERT_EQUAL(15, config.num_i16);
    TEST_ASSERT_EQUAL(16, config.num_u16);
}

TEST_CASE("read small numbers out of range (min)", "[json][read]")
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("numI8", (int)INT8_MIN - 1, doc.GetAllocator());
    doc.AddMember("numU8", -1, doc.GetAllocator());
    doc.AddMember("numI16", (int)INT16_MIN - 1, doc.GetAllocator());
    doc.AddMember("numU16", -1, doc.GetAllocator());

    print_document(doc);

    // Test
    app_config config = {};
    config.num_i8 = 7;
    config.num_u8 = 8;
    config.num_i16 = 15;
    config.num_u16 = 16;

    TEST_ASSERT_FALSE(APP_CONFIG_STATE->read(config, doc));

    // Verify
    TEST_ASSERT_EQUAL(7, config.num_i8);
    TEST_ASSERT_EQUAL(8, config.num_u8);
    TEST_ASSERT_EQUAL(15, config.num_i16);
    TEST_ASSERT_EQUAL(16, config.num_u16);
}

TEST_CASE("read invalid gpio", "[json][read]")
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("nc", (int)GPIO_NUM_NC, doc.GetAllocator());
    doc.AddMember("low", -2, doc.GetAllocator());
    doc.AddMember("high", (int)GPIO_NUM_MAX, doc.GetAllocator());
    doc.AddMember("invalid", 24, doc.GetAllocator());

    print_document(doc);

    // Test
    config_state_value<gpio_num_t> nc_state("/nc");
    config_state_value<gpio_num_t> too_low_state("/low");
    config_state_value<gpio_num_t> too_high_state("/high");
    config_state_value<gpio_num_t> invalid_state("/invalid");

    // Verify
    gpio_num_t val = GPIO_NUM_MAX;
    TEST_ASSERT_TRUE(nc_state.read(val, doc));
    TEST_ASSERT_EQUAL(GPIO_NUM_NC, val);

    TEST_ASSERT_FALSE(too_low_state.read(val, doc));
    TEST_ASSERT_FALSE(too_high_state.read(val, doc));
    TEST_ASSERT_FALSE(invalid_state.read(val, doc));
    TEST_ASSERT_EQUAL(GPIO_NUM_NC, val); // unchanged
}

TEST_CASE("write document", "[json][write]")
{
    // Data
    app_config config = {};
    config.num_i8 = 7;
    config.num_u8 = 8;
    config.num_i16 = 15;
    config.num_u16 = 16;
    config.num_i32 = 31;
    config.num_u32 = 32;
    config.num_int = 40;
    config.num_float = 42.123456;
    config.num_double = 43.123456;
    config.boolean = true;
    config.pin = GPIO_NUM_22;
    config.str = "foobar";
    config.num_list.push_back(4);
    config.num_list.push_back(8);
    config.num_list.push_back(6);
    config.str_list.emplace_back("x");
    config.str_list.emplace_back("y");

    app_config_obj obj;
    obj.ids.push_back(55);
    obj.ids.push_back(88);
    config.obj_list.push_back(obj);

    // Write
    rapidjson::Document doc;
    APP_CONFIG_STATE->write(config, doc, doc.GetAllocator());
    print_document(doc);

    // Verify
    TEST_ASSERT_EQUAL(7, doc.HasMember("numI8") && doc["numI8"].IsInt() ? doc["numI8"].GetInt() : 0);
    TEST_ASSERT_EQUAL(8, doc.HasMember("numU8") && doc["numU8"].IsInt() ? doc["numU8"].GetInt() : 0);
    TEST_ASSERT_EQUAL(15, doc.HasMember("numI16") && doc["numI16"].IsInt() ? doc["numI16"].GetInt() : 0);
    TEST_ASSERT_EQUAL(16, doc.HasMember("numU16") && doc["numU16"].IsInt() ? doc["numU16"].GetInt() : 0);
    TEST_ASSERT_EQUAL(31, doc.HasMember("numI32") && doc["numI32"].IsInt() ? doc["numI32"].GetInt() : 0);
    TEST_ASSERT_EQUAL(32, doc.HasMember("numU32") && doc["numU32"].IsInt() ? doc["numU32"].GetInt() : 0);
    TEST_ASSERT_EQUAL(40, doc.HasMember("numInt") && doc["numInt"].IsInt() ? doc["numInt"].GetInt() : 0);
    TEST_ASSERT_EQUAL(42.123456f, doc.HasMember("numFloat") && doc["numFloat"].IsFloat() ? doc["numFloat"].GetFloat() : 0.0f);
    TEST_ASSERT_EQUAL(43.123456, doc.HasMember("numDouble") && doc["numDouble"].IsDouble() ? doc["numDouble"].GetDouble() : 0.0);
    TEST_ASSERT_EQUAL(true, doc.HasMember("boolean") && doc["boolean"].IsBool() ? doc["boolean"].GetBool() : false);
    TEST_ASSERT_EQUAL(22, doc.HasMember("pin") && doc["pin"].IsInt() ? doc["pin"].GetInt() : 0);
    TEST_ASSERT_EQUAL_STRING("foobar", doc.HasMember("str") && doc["str"].IsString() ? doc["str"].GetString() : "");

    TEST_ASSERT_TRUE(doc.HasMember("numList") && doc["numList"].IsArray());
    TEST_ASSERT_EQUAL(3, doc["numList"].GetArray().Size());
    TEST_ASSERT_EQUAL(4, doc["numList"].GetArray()[0].IsInt() ? doc["numList"].GetArray()[0].GetInt() : 0);
    TEST_ASSERT_EQUAL(8, doc["numList"].GetArray()[1].IsInt() ? doc["numList"].GetArray()[1].GetInt() : 0);
    TEST_ASSERT_EQUAL(6, doc["numList"].GetArray()[2].IsInt() ? doc["numList"].GetArray()[2].GetInt() : 0);

    TEST_ASSERT_TRUE(doc.HasMember("strList") && doc["strList"].IsArray());
    TEST_ASSERT_EQUAL(2, doc["strList"].GetArray().Size());
    TEST_ASSERT_EQUAL_STRING("x", doc["strList"].GetArray()[0].IsString() ? doc["strList"].GetArray()[0].GetString() : "");
    TEST_ASSERT_EQUAL_STRING("y", doc["strList"].GetArray()[1].IsString() ? doc["strList"].GetArray()[1].GetString() : "");

    TEST_ASSERT_TRUE(doc.HasMember("objList") && doc["objList"].IsArray());
    TEST_ASSERT_EQUAL(1, doc["objList"].GetArray().Size());
    auto &obj_obj = doc["objList"].GetArray()[0];
    TEST_ASSERT_TRUE(obj_obj.HasMember("ids") && obj_obj["ids"].IsArray());
    TEST_ASSERT_EQUAL(2, obj_obj["ids"].GetArray().Size());
    TEST_ASSERT_EQUAL(55, obj_obj["ids"].GetArray()[0].IsInt() ? obj_obj["ids"].GetArray()[0].GetInt() : 0);
    TEST_ASSERT_EQUAL(88, obj_obj["ids"].GetArray()[1].IsInt() ? obj_obj["ids"].GetArray()[1].GetInt() : 0);
}

// TODO test flags

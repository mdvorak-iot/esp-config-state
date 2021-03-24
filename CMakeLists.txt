cmake_minimum_required(VERSION 3.15.0)

idf_component_register(
        SRC_DIRS src
        INCLUDE_DIRS include
        REQUIRES log nvs_flash
)

target_link_libraries(${COMPONENT_LIB} PUBLIC rapidjson)

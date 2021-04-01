cmake_minimum_required(VERSION 3.15.0)

idf_component_register(
        SRCS src/config_state_gpio.cpp src/config_state_helper.cpp
        INCLUDE_DIRS include
        REQUIRES log nvs_flash
)

target_link_libraries(${COMPONENT_LIB} PUBLIC rapidjson)

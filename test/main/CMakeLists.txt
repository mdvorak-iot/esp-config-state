cmake_minimum_required(VERSION 3.15.0)

idf_component_register(
        SRCS
        config_state_test.c
        document_test.cpp
        nvs_test.cpp
        INCLUDE_DIRS .
)

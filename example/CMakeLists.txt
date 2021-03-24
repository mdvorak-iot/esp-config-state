cmake_minimum_required(VERSION 3.15.0)
include(ExternalComponent.cmake)

# In-place use of library in this example
list(APPEND EXTRA_COMPONENT_DIRS " ${CMAKE_SOURCE_DIR}/..")

# rapidjson
# NOTE this dependency must be provided by an user
# NOTE rapidjson v1.1.0 is ancient, it must be newer, current gcc compatible, version
ExternalLibrary_Get(rapidjson https://github.com/Tencent/rapidjson 1c2c8e0)
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${rapidjson_SOURCE_DIR}/include)
add_compile_definitions(RAPIDJSON_HAS_STDSTRING=1 RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024 RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY=4 RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY=4)

# Project
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example)

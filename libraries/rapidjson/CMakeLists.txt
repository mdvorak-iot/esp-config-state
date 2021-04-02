cmake_minimum_required(VERSION 3.15.0)
include(ExternalComponent.cmake)

# This can be replaced with git submodule, however don't forget to add target_compile_definitions

# NOTE rapidjson v1.1.0 is ancient, it must be a newer version, current gcc compatible
ExternalLibrary_Get(rapidjson https://github.com/Tencent/rapidjson 1c2c8e0)
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${rapidjson_SOURCE_DIR}/include)

# Mandatory definitions
target_compile_definitions(rapidjson INTERFACE RAPIDJSON_HAS_STDSTRING=1 RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024)

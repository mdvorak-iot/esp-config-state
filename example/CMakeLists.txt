cmake_minimum_required(VERSION 3.15.0)

# In-place use of library in this example
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/..")

# rapidjson
add_subdirectory(../libraries/rapidjson ${CMAKE_CURRENT_BINARY_DIR}/rapidjson)

# Project
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(config_state_sample)

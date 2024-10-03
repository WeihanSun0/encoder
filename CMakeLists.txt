cmake_minimum_required(VERSION 2.24)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/conf")
include(compiler_conf)
include(building_output)

add_subdirectory(src)
cmake_minimum_required(VERSION 3.5.0)
project(time)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)

include_directories(.)

add_library(time SHARED
        time.c)

set_property(TARGET time PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(time yaslapi)

cmake_minimum_required (VERSION 2.6)
project (cjson_test C)

set(CMAKE_C_STANDARD 11)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(cjson cjson.c)
add_executable(cjson_test test.c)
target_link_libraries(cjson_test cjson)
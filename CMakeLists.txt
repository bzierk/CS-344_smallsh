cmake_minimum_required(VERSION 2.8.12)
project(CS-344_smallsh C)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra  -g")

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src)

cmake_minimum_required(VERSION 3.26)
project(worlds_below C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb -Wall -Werror" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-O0" CACHE STRING "" FORCE)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_executable(worlds_below)

target_sources(worlds_below
PRIVATE
    worlds_below.c
)

target_link_libraries(worlds_below SDL3::SDL3)
target_link_libraries(worlds_below SDL3_ttf::SDL3_ttf)
target_include_directories(worlds_below PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Custom command to copy a folder
add_custom_command(
    TARGET worlds_below PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources" 
    "${CMAKE_BINARY_DIR}/resources"
)

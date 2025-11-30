cmake_minimum_required(VERSION 3.10)
project(gorillas C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(gorillas src/main.c)

target_link_libraries(gorillas ${CURSES_LIBRARIES})

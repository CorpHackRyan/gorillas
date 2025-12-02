cmake_minimum_required(VERSION 3.10)
project(gorillas C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Collect all source files
file(GLOB SOURCES "src/*.c")
add_executable(gorillas ${SOURCES})

# Link SDL2 and SDL2_ttf manually
target_link_libraries(
    gorillas
    ${SDL2_LIBRARIES}
    /usr/lib/x86_64-linux-gnu/libSDL2_ttf.so
)

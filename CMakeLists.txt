cmake_minimum_required(VERSION 3.10)
project(gorillas C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Collect all source files
file(GLOB SOURCES "src/*.c")
set(BGM_MP3_SRC "${CMAKE_SOURCE_DIR}/assets/sounds/brianD_starkiller.mp3")
set(BGM_MP3_BUILD "${CMAKE_BINARY_DIR}/brianD_starkiller.mp3")
set(BGM_OBJ "${CMAKE_BINARY_DIR}/bgm_embed.o")

add_custom_command(
    OUTPUT ${BGM_OBJ}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BGM_MP3_SRC} ${BGM_MP3_BUILD}
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} ${CMAKE_LINKER} -r -b binary -o ${BGM_OBJ} brianD_starkiller.mp3
    DEPENDS ${BGM_MP3_SRC}
    COMMENT "Embedding brianD_starkiller.mp3 into binary section"
)

add_custom_target(embed_bgm ALL DEPENDS ${BGM_OBJ})

add_executable(gorillas ${SOURCES} ${BGM_OBJ})
add_dependencies(gorillas embed_bgm)
target_link_options(gorillas PRIVATE -Wl,-z,noexecstack)

# Link SDL2 and SDL2_ttf manually
target_link_libraries(
    gorillas
    ${SDL2_LIBRARIES}
    /usr/lib/x86_64-linux-gnu/libSDL2_ttf.so
    m
)

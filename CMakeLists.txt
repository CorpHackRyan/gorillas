cmake_minimum_required(VERSION 3.10)
project(gorillas C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# Find SDL2/SDL2_ttf via pkg-config (more portable on Linux distros).
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS})

# Python is used on macOS to generate embedded MP3 C source.
if(APPLE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
endif()

# Collect all source files
file(GLOB SOURCES "src/*.c")
set(BGM_MP3_SRC "${CMAKE_SOURCE_DIR}/assets/sounds/brianD_starkiller.mp3")
set(BGM_MP3_BUILD "${CMAKE_BINARY_DIR}/brianD_starkiller.mp3")
set(BGM_OBJ "${CMAKE_BINARY_DIR}/bgm_embed.o")
set(BGM_STUB "${CMAKE_BINARY_DIR}/bgm_embed.c")

if(APPLE)
    add_custom_command(
        OUTPUT ${BGM_STUB}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/embed_bgm.py ${BGM_MP3_SRC} ${BGM_STUB}
        DEPENDS ${BGM_MP3_SRC} ${CMAKE_SOURCE_DIR}/embed_bgm.py
        COMMENT "Generating embedded BGM source for macOS"
    )
    add_custom_target(embed_bgm ALL DEPENDS ${BGM_STUB})
    add_executable(gorillas ${SOURCES} ${BGM_STUB})
    add_dependencies(gorillas embed_bgm)
    target_compile_definitions(gorillas PRIVATE BGM_EMBED_C_ARRAY=1)
else()
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
endif()

# Link SDL2 and SDL2_ttf manually
target_link_libraries(
    gorillas
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    m
)

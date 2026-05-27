cmake_minimum_required(VERSION 3.12)

# Fix for CMake 4.0+ where compatibility with CMake < 3.5 is removed
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(CHIP8_emulator C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  raylib
  URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

add_executable(chip8_emulator src/main.c src/chip8.c)

target_include_directories(chip8_emulator PRIVATE src)

target_link_libraries(chip8_emulator PRIVATE raylib)

if(WIN32)
    set_target_properties(chip8_emulator PROPERTIES LINK_FLAGS "-mwindows")
endif()
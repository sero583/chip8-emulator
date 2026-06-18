cmake_minimum_required(VERSION 3.20)
project(CHIP8_Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)

add_executable(CHIP8_Emulator
    src/main.cpp
)

target_link_libraries(CHIP8_Emulator PRIVATE SDL3::SDL3)
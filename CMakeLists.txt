cmake_minimum_required(VERSION 3.20)
project(CHIP8_Emulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(CHIP8_Emulator
    src/main.cpp
)

target_link_libraries(CHIP8_Emulator PRIVATE Qt6::Widgets)

set_target_properties(CHIP8_Emulator PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
cmake_minimum_required(VERSION 3.20)
project(CHIP8_Emulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(CHIP8_Emulator
    src/main.cpp

    src/emulator/memory/Ram.h
    src/emulator/memory/Ram.cpp

    src/emulator/cpu/Cpu.h
    src/emulator/cpu/Cpu.cpp

    src/emulator/Emulator.h
    src/emulator/Emulator.cpp
)

target_include_directories(CHIP8_Emulator PRIVATE src)
target_link_libraries(CHIP8_Emulator PRIVATE Qt6::Widgets)

set_target_properties(CHIP8_Emulator PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

enable_testing()

find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch)

add_executable(CHIP8_Tests
    tests/ram_tests.cpp
    src/emulator/memory/Ram.h
    src/emulator/memory/Ram.cpp
)

target_include_directories(CHIP8_Tests PRIVATE src)
target_link_libraries(CHIP8_Tests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(CHIP8_Tests)
cmake_minimum_required(VERSION 3.20)
project(CHIP8_Emulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

qt_standard_project_setup()

add_executable(CHIP8_Emulator
    src/main.cpp

    src/emulator/memory/Ram.h
    src/emulator/memory/Ram.cpp

    src/emulator/cpu/Cpu.h
    src/emulator/cpu/Cpu.cpp

    src/emulator/display/Display.h
    src/emulator/display/Display.cpp

    src/emulator/input/InputWidget.h
    src/emulator/input/InputWidget.cpp

    src/emulator/Emulator.h
    src/emulator/Emulator.cpp

    src/emulator/Constants.h
)

target_include_directories(CHIP8_Emulator PRIVATE src)
target_link_libraries(CHIP8_Emulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(CHIP8_Emulator PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

enable_testing()

find_package(Catch2 3 CONFIG REQUIRED)
include(CTest)
include(Catch)

add_executable(CHIP8_Tests
    tests/RamTests.cpp
    src/emulator/memory/Ram.h
    src/emulator/memory/Ram.cpp

    # TODO: CPU tests
    src/emulator/cpu/Cpu.h
    src/emulator/cpu/Cpu.cpp

    tests/DisplayTests.cpp
    src/emulator/display/Display.h
    src/emulator/display/Display.cpp

    tests/InputWidgetTests.cpp
    src/emulator/input/InputWidget.h
    src/emulator/input/InputWidget.cpp

    # TODO: Emulator tests
    src/emulator/Emulator.h
    src/emulator/Emulator.cpp

    src/emulator/Constants.h

    tests/TestUtils.h
    tests/TestUtils.cpp
)

target_include_directories(CHIP8_Tests PRIVATE src)
target_link_libraries(CHIP8_Tests PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

if(WIN32)
    # Possible plugin folders
    set(QT6_PLATFORM_CANDIDATE_DIRS
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/debug/Qt6/plugins/platforms"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/tools/Qt6/plugins/platforms"
    )

    # Found plugin paths
    set(QT6_OFFSCREEN_DEBUG_DLL "")
    set(QT6_OFFSCREEN_RELEASE_DLL "")
    set(QT6_MINIMAL_DEBUG_DLL "")
    set(QT6_MINIMAL_RELEASE_DLL "")

    # Check each candidate folder
    foreach(dir IN LISTS QT6_PLATFORM_CANDIDATE_DIRS)
        # Find debug offscreen plugin
        if(NOT QT6_OFFSCREEN_DEBUG_DLL AND EXISTS "${dir}/qoffscreend.dll")
            set(QT6_OFFSCREEN_DEBUG_DLL "${dir}/qoffscreend.dll")
        endif()

        # Find release offscreen plugin
        if(NOT QT6_OFFSCREEN_RELEASE_DLL AND EXISTS "${dir}/qoffscreen.dll")
            set(QT6_OFFSCREEN_RELEASE_DLL "${dir}/qoffscreen.dll")
        endif()

        # Find debug minimal plugin
        if(NOT QT6_MINIMAL_DEBUG_DLL AND EXISTS "${dir}/qminimald.dll")
            set(QT6_MINIMAL_DEBUG_DLL "${dir}/qminimald.dll")
        endif()

        # Find release minimal plugin
        if(NOT QT6_MINIMAL_RELEASE_DLL AND EXISTS "${dir}/qminimal.dll")
            set(QT6_MINIMAL_RELEASE_DLL "${dir}/qminimal.dll")
        endif()
    endforeach()

    # Create plugin folder for tests
    add_custom_command(TARGET CHIP8_Tests POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:CHIP8_Tests>/platforms"
        COMMENT "Ensuring Qt platforms directory exists for CHIP8_Tests"
    )

    # Copy debug offscreen plugin
    if(QT6_OFFSCREEN_DEBUG_DLL)
        add_custom_command(TARGET CHIP8_Tests POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${QT6_OFFSCREEN_DEBUG_DLL}"
                "$<TARGET_FILE_DIR:CHIP8_Tests>/platforms/qoffscreend.dll"
            COMMENT "Copying qoffscreend.dll for Debug headless Qt tests"
        )
    endif()

    # Copy release offscreen plugin
    if(QT6_OFFSCREEN_RELEASE_DLL)
        add_custom_command(TARGET CHIP8_Tests POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${QT6_OFFSCREEN_RELEASE_DLL}"
                "$<TARGET_FILE_DIR:CHIP8_Tests>/platforms/qoffscreen.dll"
            COMMENT "Copying qoffscreen.dll for Release headless Qt tests"
        )
    endif()

    # Copy debug minimal fallback plugin
    if(QT6_MINIMAL_DEBUG_DLL)
        add_custom_command(TARGET CHIP8_Tests POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${QT6_MINIMAL_DEBUG_DLL}"
                "$<TARGET_FILE_DIR:CHIP8_Tests>/platforms/qminimald.dll"
            COMMENT "Copying qminimald.dll as fallback Debug Qt platform"
        )
    endif()

    # Copy release minimal fallback plugin
    if(QT6_MINIMAL_RELEASE_DLL)
        add_custom_command(TARGET CHIP8_Tests POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${QT6_MINIMAL_RELEASE_DLL}"
                "$<TARGET_FILE_DIR:CHIP8_Tests>/platforms/qminimal.dll"
            COMMENT "Copying qminimal.dll as fallback Release Qt platform"
        )
    endif()
endif()

# Discover tests before running
set(CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

catch_discover_tests(CHIP8_Tests
    WORKING_DIRECTORY $<TARGET_FILE_DIR:CHIP8_Tests>
    DISCOVERY_MODE PRE_TEST
)

if(WIN32)
    # Get discovered tests
    get_property(CHIP8_DISCOVERED_TESTS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY TESTS)

    # Set Qt env vars for test runs
    foreach(test_name IN LISTS CHIP8_DISCOVERED_TESTS)
        # Only for CHIP8_Tests entries
        if(test_name MATCHES "^CHIP8_Tests")
            set_tests_properties(${test_name} PROPERTIES
                # Enable offscreen, disable debug plugins and filter out warnings to avoid spamming test output with
                #e. g. font warnings since we don't rely on golden testing
                ENVIRONMENT
                    "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<SHELL_PATH:$<TARGET_FILE_DIR:CHIP8_Tests>/platforms>;QT_DEBUG_PLUGINS=0;QT_LOGGING_RULES=*.warning=false"
            )
        endif()
    endforeach()
endif()
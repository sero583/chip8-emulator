#pragma once

/**
 * @file Constants.h
 * @brief Contains global constants and definitions used across the emulator.
 */

namespace CpuProperties {
    /**
     * @brief Interval in milliseconds for the CPU timer, controlling how often the CPU executes cycles.
     * 
     * This value can be adjusted to speed up or slow down the emulation. A lower value results in faster emulation, while a higher value results in slower emulation.
     * The default value of 1 ms is chosen to provide a good balance between performance and accuracy for most ROMs, but it can be tweaked as needed for specific use cases or testing purposes.
     */
    constexpr int CPU_TIMER_INTERVAL_MS = 1;
}

namespace MemoryProperties {
    /**
     * @brief Minimum valid raw memory address.
     */
    constexpr uint16_t RAW_MEM_MIN = 0x000;
    /**
     * @brief Maximum valid raw memory address.
     */
    constexpr uint16_t RAW_MEM_MAX = 0xFFF;
    /**
     * @brief Minimum valid program memory address.
     */
    constexpr uint16_t PROGRAM_MEM_MIN = 0x200;
    /**
     * @brief Total size of the memory in bytes.
     */
    constexpr std::size_t MEMORY_SIZE = 4096;
}

namespace DisplayProperties {
    /**
     * @brief Width of the CHIP-8 display in pixels.
     */
    constexpr int CHIP8_DISPLAY_WIDTH = 64;
    /**
     * @brief Height of the CHIP-8 display in pixels.
     */
    constexpr int CHIP8_DISPLAY_HEIGHT = 32;
}

namespace InputProperties {
    /**
     * @brief Layout of the input buttons, mapping each button label to its corresponding key index.
     * 
     * Note: Could have been done without a pair-array, since labels are the hexadecimal representation - however, if I ever wanted in the future to support custom layouts with custom non-hex based labels,
     * this would make it easier to do so.
     */
    constexpr std::array<std::pair<char, int>, 16> buttonLayout = {{
        {'0', 0x0}, {'1', 0x1}, {'2', 0x2}, {'3', 0x3},
        {'4', 0x4}, {'5', 0x5}, {'6', 0x6}, {'7', 0x7},
        {'8', 0x8}, {'9', 0x9}, {'A', 0xA}, {'B', 0xB},
        {'C', 0xC}, {'D', 0xD}, {'E', 0xE}, {'F', 0xF}
    }};
    /**
     * @brief Number of columns in the button layout, used for arranging buttons in a grid.
     */
    constexpr int rowWidth = 4;
}
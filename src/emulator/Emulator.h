#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <QString>

#include "emulator/Constants.h"
#include "emulator/cpu/Cpu.h"
#include "emulator/memory/Ram.h"

class Emulator {
private:
    Ram ram;
    Cpu cpu;
    std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT> displayBuffer{};

public:
    Emulator() : ram(), cpu(ram, displayBuffer) {}


    /**
     * Resets the emulator's state, including RAM and any other components.
     * Currently, this only resets RAM, but it can be expanded in the future
     * to include other subsystems as needed.
     * 
     * @param restoreRom If true, the currently loaded ROM will be restored from memory, not filesystem.
     * @param wipeRomCopy If true, the internal ROM copy will be cleared during reset.
     * @throws std::invalid_argument If restoreRom and wipeRomCopy are both enabled, since it would make restoring impossible.
     */
    void reset(bool restoreRom = false, bool wipeRomCopy = false);

    /**
     * Loads ROM data into the emulator's memory starting at the default program memory address (0x200).
     * 
     * @param rom ROM payload to copy into memory.
     */
    void loadROM(const std::vector<uint8_t>& rom);

    /**
     * Loads ROM data from a file into the emulator's memory starting at the default program memory address (0x200).
     * 
     * @param filePath Path to the ROM file.
     * @return True if the ROM was loaded successfully, false otherwise.
     */
    bool loadROMFromFile(const QString& filePath);

    /**
     * Triggers a CPU cycle.
     * 
     * @return True if the CPU requests a display update after the cycle, false otherwise.
     */
    bool cycleCpu();

    /**
     * Resets the CPU.
     */
    void resetCpu();

    /**
     * Returns the display buffer representing the current state of the emulator's display.
     * The buffer is a flat array of bytes, where each byte represents a pixel (0 for off, 1 for on) in the 64x32 CHIP-8 display.
     * @return A reference to the display buffer array.
     */
    const std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& getDisplayBuffer() const;
};
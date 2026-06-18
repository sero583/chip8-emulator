#pragma once

#include <cstdint>
#include <vector>

#include <QString>

#include "emulator/memory/Ram.h"
#include "emulator/cpu/Cpu.h"

class Emulator {
private:
    Ram ram;
    Cpu cpu;

public:
    Emulator() : ram(), cpu(ram) {}

    /**
     * Resets the emulator's state, including RAM and any other components.
     * Currently, this only resets RAM, but it can be expanded in the future
     * to include other subsystems as needed.
     */
    void reset();

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
     */
    void cycleCpu();
};
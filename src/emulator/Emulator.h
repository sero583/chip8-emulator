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
    std::array<bool, 16> keyState{};

public:
    /**
     * @brief Constructs a new Emulator instance.
     */
    Emulator() : ram(), cpu(*this, displayBuffer) {}

    /**
     * @brief Resets the emulator's state.
     * 
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
     * @brief Gets a reference to the emulator's RAM.
     * 
     * @return A reference to the emulator's RAM instance.
     */
    Ram getRam() const;

    /**
     * @brief Loads ROM data into memory.
     * 
     * Loads ROM data into the emulator's memory starting at the default program memory address (0x200).
     * 
     * @param rom ROM payload to copy into memory.
     */
    void loadROM(const std::vector<uint8_t>& rom);

    /**
     * @brief Loads ROM from file.
     * 
     * Loads ROM data from a file into the emulator's memory starting at the default program memory address (0x200).
     * 
     * @param filePath Path to the ROM file.
     * @return True if the ROM was loaded successfully, false otherwise.
     */
    bool loadROMFromFile(const QString& filePath);

    /**
     * @brief Triggers a CPU cycle.
     * 
     * @return True if the CPU requests a display update after the cycle, false otherwise.
     */
    bool cycleCpu();

    /**
     * @brief Resets the CPU.
     */
    void resetCpu();

    /**
     * @brief Gets the current state of the display buffer.
     * 
     * Returns the display buffer representing the current state of the emulator's display.
     * The buffer is a flat array of bytes, where each byte represents a pixel (0 for off, 1 for on) in the 64x32 CHIP-8 display.
     * 
     * @return A reference to the display buffer array.
     */
    const std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& getDisplayBuffer() const;

    /**
     * @brief Sets the state of a CHIP-8 keypad key.
     * 
     * @param key The index of the key (0x0 to 0xF).
     * @param pressed True if the key is pressed, false if it is released.
     * @throws std::out_of_range If the key index is outside the valid range (0x0 to 0xF).
     */
    void setKeyState(uint8_t key, bool pressed);

    /**
     * @brief Gets the current state of a CHIP-8 keypad key.
     * 
     * @param key The index of the key (0x0 to 0xF).
     * @return True if the key is currently pressed, false otherwise.
     * @throws std::out_of_range If the key index is outside the valid range (0x0 to 0xF).
     */
    bool getKeyState(uint8_t key) const;
};
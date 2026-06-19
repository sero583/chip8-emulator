#pragma once

#include <array>
#include <cstdint>

#include <stdexcept>
#include <string>

#include "emulator/Constants.h"
#include "emulator/memory/Ram.h"

class Cpu {
public:
    Cpu(Ram& ramRef, std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& displayBufferRef) : ramRef(ramRef), displayBufferRef(displayBufferRef) {}

    /**
     * Resets the CPU's state, including registers, program counter, stack pointer, and any other components.
     * Currently, this is a placeholder method, but it can be expanded in the future to include actual CPU state reset logic.
     */
    void reset();

    /**
     * Fetches the next opcode from memory.
     * 
     * @param incrementPc Increments program counter by two when enabled after fetching.
     * @return The 16-bit opcode fetched from memory.
     */
    uint16_t fetchOpcode(bool incrementPc = true);

    /**
     * Performs a single CPU cycle, including fetching, decoding, and executing the next instruction.
     * 
     * @return True if the executed instruction requests a display update, false otherwise.
     */
    bool cycle();

    /**
     * Gets the current value of the program counter.
     * @return The current program counter value.
     */
    uint16_t getProgramCounter() const;

    /**
     * Gets the current value of a general purpose register.
     * 
     * @param index The index of the register to retrieve.
     * @return The current value of the specified register.
     */
    uint8_t getRegister(uint8_t index) const;

    /**
     * Executes the given opcode.
     * 
     * @param opcode The opcode to execute.
     * @return True if the executed instruction requests a display update, false otherwise.
     */
    bool executeOpcode(uint16_t opcode);

private:
    // Reference to RAM
    Ram& ramRef;
    // Reference to display buffer for opcodes that manipulate the display
    std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& displayBufferRef;

    // General purpose registers V0 to VF
    std::array<uint8_t, 16> V{};
    // Index register
    uint16_t i = 0;
    // Program counter
    uint16_t pc = Ram::PROGRAM_MEM_MIN;
    // Stack pointer
    uint8_t sp = 0;
    // Stack (up to 16 levels)
    std::array<uint16_t, 16> stack{};
    // Delay timer
    uint8_t delayTimer = 0;
    // Sound timer
    uint8_t soundTimer = 0;
    // Current opcode
    uint16_t opcode = 0;

    // Opcode generic helpers
    void copyVyToVx(uint8_t x, uint8_t y) {
        V[x] = V[y];
    }

    void orRegisters(uint8_t x, uint8_t y) {
        V[x] |= V[y];
    }

    void andRegisters(uint8_t x, uint8_t y) {
        V[x] &= V[y];
    }

    void xorRegisters(uint8_t x, uint8_t y) {
        V[x] ^= V[y];
    }

    void addRegisters(uint8_t x, uint8_t y) {
        // Add register values using 16 bit to detect overflow / carry
        uint16_t sum = V[x] + V[y];
        // Set VF to 1 if result does not fit into 8 bit, otherwise 0
        V[0xF] = (sum > 0xFF) ? 1 : 0;
        // Store only the lower 8 bits back into Vx
        V[x] = static_cast<uint8_t>(sum & 0x00FF);
    }
};
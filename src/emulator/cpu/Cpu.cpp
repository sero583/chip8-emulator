#include "Cpu.h"

void Cpu::reset() {
    V.fill(0);
    i = 0;
    pc = Ram::PROGRAM_MEM_MIN;
    sp = 0;
    stack.fill(0);
    delayTimer = 0;
    soundTimer = 0;
    opcode = 0;
}

uint16_t Cpu::fetchOpcode(bool incrementPc) {
    opcode = ramRef.readOpcode(pc);

    // increment to next opcode when enabled
    if(incrementPc) {
        pc += 2;
    }

    return opcode;
}

bool Cpu::cycle() {
    return executeOpcode(fetchOpcode());
}

uint16_t Cpu::getProgramCounter() const {
    return pc;
}

uint8_t Cpu::getRegister(uint8_t index) const {
    return V[index];
}


bool Cpu::executeOpcode(uint16_t opcode) {
     // 1. upper nibble is instruction
    uint16_t instruction = opcode & 0xF000;
    // 2. register index is second nibble, need to shift by 8 to get actual number and remove remaints of two nibbles
    uint8_t x = (opcode & 0x0F00) >> 8;
    // 3. register index 2 is third nibble, need to shift by 4 to remove remaints of one nibble 
    uint8_t y = (opcode & 0x00F0) >> 4;
    // 3. N value 8 bit
    uint8_t n = opcode & 0x000F;
    // 4. NN value, last 8 bits
    uint8_t nn = opcode & 0x00FF;
    // 5. NNN value 12 bit -> use 16 bit C++ data type
    uint16_t nnn = opcode & 0x0FFF;

    switch(instruction) {
        case 0x0000:
            if (opcode == 0x00E0) {
                // 00E0: clear display
                displayBufferRef.fill(0);
                return true;
            } else if (opcode == 0x00EE) {
                // 00EE: return from subroutine
                sp--;
                pc = stack[sp];
            } else {
                throw std::runtime_error("Unknown 0x0000 opcode: " + std::to_string(opcode));
            }
        break;
        // Set program counter to NNN
        case 0x1000:
            pc = nnn;
        break;
        // Sets Vx to NN
        case 0x6000: 
            V[x] = nn;
        break;
        // Add NN to Vx without affecting carry flag
        case 0x7000:
            V[x] += nn;
        break;
        case 0x8000:
            switch(n) {
                case 0x0: copyVyToVx(x, y); break;
                case 0x1: orRegisters(x, y); break;
                case 0x2: andRegisters(x, y); break;
                case 0x3: xorRegisters(x, y); break;
                case 0x4: addRegisters(x, y); break;
                default: throw std::runtime_error("Unsupported 8XY? opcode: " + std::to_string(opcode));
            }
        break;
        // Set index register I to address NNN
        case 0xA000:
            i = nnn;
        break;
        // Draw sprite on Position (Vx, Vy) at height N from memory address I, VF shows collision
        case 0xD000: {
            // DXYN:
            // Draw a sprite at coordinate (VX, VY).
            // The sprite is stored starting at memory address I.
            // The sprite is always 8 pixels wide and N pixels tall.
            // Drawing uses XOR.
            // VF is set to 1 if at least one pixel is turned off during drawing.

            uint8_t xIndex = (opcode & 0x0F00) >> 8;
            uint8_t yIndex = (opcode & 0x00F0) >> 4;
            uint8_t height = opcode & 0x000F;

            uint8_t xPos = V[xIndex];
            uint8_t yPos = V[yIndex];

            // Clear the collision flag before drawing.
            V[0xF] = 0;

            // Each sprite row is 1 byte in memory.
            for (uint8_t row = 0; row < height; ++row) {
                uint8_t spriteByte = ramRef.read(i + row);

                // A sprite is always 8 bits wide.
                for (uint8_t col = 0; col < 8; ++col) {

                    // The most significant bit is on the left.
                    // col = 0 checks bit 7, col = 1 checks bit 6, etc.
                    uint8_t spritePixel = spriteByte & (0x80 >> col);

                    // Only draw if the sprite bit is set.
                    if (spritePixel) {
                        // Wraparound on a 64x32 screen.
                        uint8_t screenX = (xPos + col) % 64;
                        uint8_t screenY = (yPos + row) % 32;

                        // Convert 2D coordinates to 1D buffer index.
                        size_t bufferIndex = screenY * 64 + screenX;

                        // If the pixel at this position is already 1,
                        // and we XOR with 1 again, it will be cleared.
                        // This is a collision.
                        if (displayBufferRef[bufferIndex] == 1) {
                            V[0xF] = 1;
                        }

                        // XOR drawing:
                        // 0 ^ 1 = 1  -> pixel turns on
                        // 1 ^ 1 = 0  -> pixel turns off
                        displayBufferRef[bufferIndex] ^= 1;
                    }
                }
            }
            return true;
        }
        default: throw std::runtime_error("Unsupported opcode: " + std::to_string(opcode));
    }
    return false;
}

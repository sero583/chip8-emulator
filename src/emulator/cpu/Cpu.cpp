#include <cstdlib> 
#include <sstream>
#include <iomanip>

#include "emulator/cpu/Cpu.h"
#include "emulator/Emulator.h"

namespace {
    [[noreturn]] void throwUnknownOpcodeError(uint16_t opcode, const char* prefix = "") {
        std::ostringstream oss;
        oss << prefix
            << "Unsupported opcode: " << opcode
            << " (0x" << std::uppercase << std::hex
            << std::setw(4) << std::setfill('0') << opcode << ")";
        throw std::runtime_error(oss.str());
    }
}

void Cpu::reset() {
    V.fill(0);
    i = 0;
    pc = MemoryProperties::PROGRAM_MEM_MIN;
    sp = 0;
    stack.fill(0);
    delayTimer = 0;
    soundTimer = 0;
    opcode = 0;
    idleState = IdleState::notIdle;
}

uint16_t Cpu::fetchOpcode(bool incrementPc) {
    opcode = emulatorRef.getRam().readOpcode(pc);

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
        case 0x0000: {
            if (opcode == 0x00E0) {
                // 00E0: clear display
                displayBufferRef.fill(0);
                return true;
            } else if (opcode == 0x00EE) {
                if(sp==0) {
                    throw std::runtime_error("Stack underflow on 00EE: ROM returned from an empty stack");
                }
                pc = stack[--sp];
                break;
            }
            // 0NNN: Bonus task - only OGs implement an RCA 1802 emulator... :)
            throwUnknownOpcodeError(opcode);
        }
        // 1NNN: Set program counter to NNN
        case 0x1000:
            pc = nnn;
        break;
        // 0x2NNN: Call subroutine from NNN, push current (+2, in our case by one since fetch already increments) PC to the stack, increment SP, set PC = NNN.
        case 0x2000:
            if (sp >= stack.size()) {
                throw std::runtime_error("Stack overflow on 2NNN");
            }
            stack[sp++] = pc;
            pc = nnn;
        break;
        // 3XNN: Skip the following instruction if the value of register Vx equals NN.
        case 0x3000:
            if(V[x]==nn) {
                pc += 2;
            }
        break;
        // 4XNN: Skip the following instruction if the value of register Vx is not equal to NN.
        case 0x4000:
            if(V[x]!=nn) {
                pc += 2;
            }
        break;
        // 5XY0: Skip next instruction, if value in register Vx is equal to the value in register Vy and n = 0
        case 0x5000:
            if(n==0 && V[x]==V[y]) {
                pc += 2;
            }
        break;
        // 6XNN: Sets Vx to NN
        case 0x6000: 
            V[x] = nn;
        break;
        // 7XNN: Add NN to Vx without affecting carry flag
        case 0x7000:
            V[x] += nn;
        break;
        // 8-group: Arithmetic & bitwise operations
        case 0x8000: {
            // common actions
            switch(n) {
                case 0x0: copyVyToVx(x, y); return false;
                case 0x1: orRegisters(x, y); return false;
                case 0x2: andRegisters(x, y); return false;
                case 0x3: xorRegisters(x, y); return false;
                case 0x4: addRegisters(x, y); return false;
                // 8XY5: Set VF on borrow behavior, then Vx = Vx - Vy
                case 0x5: 
                    V[0xF] = (V[x] >= V[y]) ? 1 : 0;  // 0 on borrow, 1 on no borrow
                    V[x] -= V[y];
                return false;
                // 8XY6: VF = least-significant bit of Vy before the shift, then Vx = Vy >> 1 
                case 0x6:
                    V[0xF] = V[y] & 1;
                    V[x] = V[y] >> 1;
                return false;
                // 8XY7: Vx = Vy - Vx, set VF on borrow behavior.
                case 0x7:
                    V[0xF] = (V[y] >= V[x]) ? 1 : 0;  // 0 on borrow, 1 on no borrow
                    V[x] = V[y] - V[x];
                return false;
                // 8XYE: VF = most-significant bit of Vy before the shift, then Vx = Vy << 1
                case 0xE:
                    V[0xF] = V[y] >> 7;
                    V[x] = V[y] << 1;
                return false;
            }
            throwUnknownOpcodeError(opcode);
        }
        // 9XY0: Counterpart of 5XY0 - when Vx != Vy and n = 0, skip instruction
        case 0x9000:
            if(n==0 && V[x]!=V[y]) {
                pc += 2;
            }
        break;
        // ANNN: Set index register I to address NNN
        case 0xA000:
            i = nnn;
        break;
        // BNNN: Jump to location NNN + V0
        case 0xB000:
            pc = nnn + V[0];
        break;
        // CXNN: Chip 8's randomizer - Set Vx = random byte AND NN
        case 0xC000:
            V[x] = static_cast<uint8_t>(rand()) & nn;
        break;
        // DXYN: Draw sprite on Position (Vx, Vy) at height N from memory address I, VF shows collision
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
                uint8_t spriteByte = emulatorRef.getRam().read(i + row);

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
        case 0xE000: {
            // EX9E: skip if pressed, EXA1: skip if not pressed
            bool keyPressed = emulatorRef.getKeyState(V[x]);

            if((y == 0x9 && n == 0xE && keyPressed) || (y == 0xA && n == 0x1 && !keyPressed)) {
                pc += 2;
                return false;
            }
            throwUnknownOpcodeError(opcode);
        }
        case 0xF000: {
            if(nn==0x0A) {
                // FX0A: Wait for ANY keypress.
                // When key is pressed, store key's value to register Vx
                // Idle CPU until key press occurs.

                // TODO: Need more efficient implementation, which will trigger on key release not press
                // Since this is simpler and works for now, we keep it.

                bool foundPressedKey = false;

                for(uint8_t key = 0; key < 16; ++key) {
                    if(emulatorRef.getKeyState(key)) {
                        V[x] = key;
                        foundPressedKey = true;
                        break;
                    }
                }

                if(!foundPressedKey) {
                    idleState = IdleState::waitingForKey;

                    pc -= 2; // remain on same opcode until key is pressed
                } else {
                    idleState = IdleState::notIdle;
                }

                return false;
            } else if(nn==0x1E) {
                // FX1E: Add value in register Vx to the index register I, and store result back in I
                i += V[x];
                return false;
            }
            // TODO: more opcodes

            // Remove below and enable again: throwUnknownOpcodeError(opcode); just a bypass for now for testing, since we haven't implemented all opcodes yet
            return false;
        }
        default: throwUnknownOpcodeError(opcode);
    }
    return false;
}

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

void Cpu::cycle() {
    executeOpcode(fetchOpcode());
}

uint16_t Cpu::getProgramCounter() const {
    return pc;
}

uint8_t Cpu::getRegister(uint8_t index) const {
    return V[index];
}


void Cpu::executeOpcode(uint16_t opcode) {
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
                // TODO: implement screen clear
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
        case 0xD000:
            // TODO: Implement display & drawing...
        break;
        default:
            throw std::runtime_error("Unsupported opcode: " + std::to_string(opcode));
    }
}

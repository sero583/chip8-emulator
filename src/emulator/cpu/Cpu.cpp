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
    // TODO: Implement opcode execution logic
}

#include <cstdint>
#include <stdexcept>
#include <vector>

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QIODevice>

#include "Emulator.h"

void Emulator::reset(bool restoreRom, bool wipeRomCopy) {
    if(restoreRom && wipeRomCopy) {
        throw std::invalid_argument("Cannot wipe ROM copy when restoreRom is enabled, as it would make restoring impossible.");
    }

    displayBuffer.fill(0);
    ram.reset(wipeRomCopy);
    cpu.reset();
    
    if(restoreRom) {
        ram.restoreRomCopy();
    }
}

void Emulator::loadROM(const std::vector<uint8_t>& rom) {
    ram.loadROM(rom);
}

bool Emulator::loadROMFromFile(const QString& filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray data = file.readAll();

    std::vector<uint8_t> rom(data.begin(), data.end());

    ram.loadROM(rom);
    return true;
}

bool Emulator::cycleCpu() {
    return cpu.cycle();
}

void Emulator::resetCpu() {
    cpu.reset();
}

const std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& Emulator::getDisplayBuffer() const {
    return displayBuffer;
}
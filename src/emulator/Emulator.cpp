#include "Emulator.h"

#include <vector>
#include <cstdint>

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QIODevice>

void Emulator::reset() {
    ram.reset();
    cpu.reset();
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

void Emulator::cycleCpu() {
    cpu.cycle();
}
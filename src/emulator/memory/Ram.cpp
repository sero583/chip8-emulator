#include "Ram.h"

#include <stdexcept>
#include <string>

constexpr uint16_t RAW_MEM_MIN = 0x000;
constexpr uint16_t RAW_MEM_MAX = 0xFFF;
constexpr uint16_t PROGRAM_MEM_MIN = 0x200;

Ram::Ram() {}

void Ram::reset() {
    // Zero-fill to reset
    memory.fill(0);
}

Ram::AddressValidationResult Ram::validateAddressInRange(
    uint16_t address,
    uint16_t min,
    uint16_t max,
    bool throwException,
    const char* label
) const {
    if(address < min) {
        if(throwException) {
            throw std::runtime_error(std::string(label) + " value cannot be lower than " + std::to_string(min) + ".");
        }
        return Ram::AddressValidationResult::TooSmall;
    }

    if(address > max) {
        if(throwException) {
            throw std::runtime_error(std::string(label) + " value cannot be greater than " + std::to_string(max) + ".");
        }
        return Ram::AddressValidationResult::TooLarge;
    }

    return Ram::AddressValidationResult::InBounds;
}

Ram::AddressValidationResult Ram::validateRawMemoryAddress(uint16_t address, bool throwException) const {
    return validateAddressInRange(address, RAW_MEM_MIN, RAW_MEM_MAX, throwException, "address");
}

Ram::AddressValidationResult Ram::validateProgramMemoryAddress(uint16_t address, bool throwException) const {
    return validateAddressInRange(address, PROGRAM_MEM_MIN, RAW_MEM_MAX, throwException, "startAddress");
}

uint8_t Ram::read(uint16_t address) const {
    validateRawMemoryAddress(address, true);
    return memory[address];
}

void Ram::write(uint16_t address, uint8_t value) {
    validateRawMemoryAddress(address, true);
    memory[address] = value;
}

uint16_t Ram::readOpcode(uint16_t address) const {
    validateRawMemoryAddress(address, true);
    validateRawMemoryAddress(address + 1, true);

    // big endian
    return (static_cast<uint16_t>(memory[address]) << 8) | static_cast<uint16_t>(memory[address + 1]);
}

void Ram::loadROM(const std::vector<uint8_t>& rom, uint16_t startAddress) {
    // Check program memory boundaries
    validateProgramMemoryAddress(startAddress, true);

    size_t romSize = rom.size();

    // Check if ROM can fit from the desired startAddress
    if((romSize + startAddress) > memory.size()) {
        throw std::runtime_error("ROM is exceeding memory boundary.");
    }

    // Write ROM into memory
    for(size_t i = 0; i < rom.size(); i++) {
        memory[startAddress + i] = rom[i];
    }
}
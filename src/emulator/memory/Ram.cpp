#include "Ram.h"

#include <stdexcept>
#include <string>

Ram::Ram() {}

void Ram::reset(bool wipeRomCopy) {
    // Zero-fill to reset
    memory.fill(0);

    // Clear rom copy when enabled
    if(wipeRomCopy) {
        romCopy.fill(0);
    }
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
    return validateAddressInRange(address, Ram::RAW_MEM_MIN, Ram::RAW_MEM_MAX, throwException, "address");
}

Ram::AddressValidationResult Ram::validateProgramMemoryAddress(uint16_t address, bool throwException) const {
    return validateAddressInRange(address, Ram::PROGRAM_MEM_MIN, Ram::RAW_MEM_MAX, throwException, "startAddress");
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

    size_t romSize = loadedRomSize = rom.size();

    // Check if ROM can fit from the desired startAddress
    if((romSize + startAddress) > memory.size()) {
        throw std::runtime_error("ROM is exceeding memory boundary.");
    }

    // Store the start address of the program memory region where the ROM is loaded, so we can use it for validating ROM loading and resetting.
    programMemoryStart = startAddress;

    // Write ROM into memory
    for(size_t i = 0; i < rom.size(); i++) {
        memory[startAddress + i] = rom[i];
    }

    // Store a copy of the loaded ROM for resetting purposes, since we don't want to rely on the filesystem after loading a ROM.
    for(size_t i = 0; i < rom.size(); i++) {
        romCopy[i] = rom[i];
    }
}

/**
 * Restores the currently loaded ROM from the internal copy, allowing for a reset without needing to read from the filesystem again.
 * This is useful for resetting the emulator state while keeping the same ROM loaded.
 */
void Ram::restoreRomCopy() {
    // Clear all memory to ensure a clean state before restoring the ROM copy, since we want to make sure we don't have any residual data from the previous state that could interfere with the ROM's execution.
    memory.fill(0);

    // Restore the ROM copy into memory starting at the program memory start address, using the loaded ROM size to ensure we only restore the relevant portion of the ROM copy.
    for (size_t i = 0; i < loadedRomSize; i++) {
        memory[programMemoryStart + i] = romCopy[i];
    }
}
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "emulator/Constants.h"

class Ram {
public:
    /**
     * Result of a memory address bounds check.
     */
    enum class AddressValidationResult {
        InBounds,   ///< Address is within the valid range.
        TooSmall,   ///< Address is below the valid range.
        TooLarge    ///< Address exceeds the valid range.
    };

    Ram();

    /**
     * Resets the entire memory to zero.
     * 
     * @param wipeRomCopy If true, also clears the internal ROM copy used for restoring the loaded ROM.
     */
    void reset(bool wipeRomCopy = true);

    /**
     * Reads the byte at the given raw memory address.
     *
     * @param address Raw memory address to read from.
     * @return The byte stored at the given address.
     * @throws std::runtime_error If the address is outside the valid raw memory range.
     */
    uint8_t read(uint16_t address) const;

    /**
     * Writes a byte to the given raw memory address.
     *
     * @param address Raw memory address to write to.
     * @param value Byte value to store.
     * @throws std::runtime_error If the address is outside the valid raw memory range.
     */
    void write(uint16_t address, uint8_t value);

    /**
     * Reads a 2-byte opcode from the given raw memory address.
     *
     * CHIP-8 opcodes are stored in big-endian order, so the byte at
     * the given address is treated as the high byte and the next byte
     * as the low byte.
     *
     * @param address Raw memory address of the opcode.
     * @return The decoded 16-bit opcode.
     * @throws std::runtime_error If the opcode would exceed the valid raw memory range.
     */
    uint16_t readOpcode(uint16_t address) const;

    /**
     * Loads ROM data into program memory starting at the given address.
     *
     * @param rom ROM payload to copy into memory.
     * @param startAddress Program memory start address.
     * @throws std::runtime_error If the start address is outside the valid program memory range
     *         or if the ROM would exceed available memory.
     */
    void loadROM(const std::vector<uint8_t>& rom, uint16_t startAddress = 0x200);

    /**
     * Restores the currently loaded ROM from the internal copy, allowing for a reset without needing to read from the filesystem again.
     * This is useful for resetting the emulator state while keeping the same ROM loaded.
     */
    void restoreRomCopy();

private:
    /**
     * Validates whether the given address is inside the specified address range.
     *
     * @param address Address to validate.
     * @param min Minimum allowed address.
     * @param max Maximum allowed address.
     * @param throwException If true, throws an exception when the address is invalid.
     * @param label Human-readable label used in exception messages.
     * @return AddressValidationResult::InBounds if the address is valid,
     *         AddressValidationResult::TooSmall if it is below the minimum,
     *         or AddressValidationResult::TooLarge if it exceeds the maximum.
     *
     * @throws std::runtime_error If throwException is true and the address is invalid.
     */
    AddressValidationResult validateAddressInRange(
        uint16_t address,
        uint16_t min,
        uint16_t max,
        bool throwException,
        const char* label
    ) const;

    /**
     * Validates whether the given address is inside the raw CHIP-8 memory range.
     *
     * @param address Raw memory address to validate.
     * @param throwException If true, throws an exception when the address is invalid.
     * @return The validation result for the given raw memory address.
     *
     * @throws std::runtime_error If throwException is true and the address is invalid.
     */
    AddressValidationResult validateRawMemoryAddress(uint16_t address, bool throwException = false) const;

    /**
     * Validates whether the given address is inside the CHIP-8 program memory range.
     *
     * @param address Program memory address to validate.
     * @param throwException If true, throws an exception when the address is invalid.
     * @return The validation result for the given program memory address.
     *
     * @throws std::runtime_error If throwException is true and the address is invalid.
     */
    AddressValidationResult validateProgramMemoryAddress(uint16_t address, bool throwException = false) const;

    // RAM must start zero-initialized.
    std::array<uint8_t, MemoryProperties::MEMORY_SIZE> memory{};
    // Start address of the default program memory region where ROMs are loaded, used for validating ROM loading and resetting.
    uint16_t programMemoryStart;
    // Loaded ROM size, used for validating ROM loading and resetting.
    size_t loadedRomSize;
    // ROM copy for resetting without needing to read from filesystem again, since its unreliable as files could be moved or deleted.
    std::array<uint8_t, MemoryProperties::MEMORY_SIZE> romCopy{};
};
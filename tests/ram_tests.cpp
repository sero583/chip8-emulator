#include <catch2/catch_all.hpp>
#include "emulator/memory/Ram.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
    constexpr uint8_t DefaultByte = 0x00;

    constexpr uint8_t FirstTestByte = 0x12;
    constexpr uint8_t SecondTestByte = 0xAB;
    constexpr uint8_t ThirdTestByte = 0xFE;

    constexpr uint8_t OpcodeHighByte = 0xAB;
    constexpr uint8_t OpcodeLowByte = 0xCD;
    constexpr uint16_t ExpectedOpcode = 0xABCD;

    constexpr uint16_t LowestRawAddress = Ram::RAW_MEM_MIN;
    constexpr uint16_t HighestRawAddress = Ram::RAW_MEM_MAX;
    constexpr uint16_t ProgramStartAddress = Ram::PROGRAM_MEM_MIN;
    constexpr uint16_t LastOpcodeStartAddress = Ram::RAW_MEM_MAX - 1;
    constexpr uint16_t InvalidHighAddress = Ram::RAW_MEM_MAX + 1;
    constexpr uint16_t InvalidProgramAddress = Ram::PROGRAM_MEM_MIN - 1;

    constexpr uint8_t RomByte1 = 0xDE;
    constexpr uint8_t RomByte2 = 0xAD;
    constexpr uint8_t RomByte3 = 0xBE;
    constexpr uint8_t RomByte4 = 0xEF;
}

TEST_CASE("RAM is zero-initialized after reset", "[ram]") {
    Ram ram;

    ram.write(LowestRawAddress, FirstTestByte);
    ram.write(ProgramStartAddress, SecondTestByte);
    ram.write(HighestRawAddress, ThirdTestByte);

    ram.reset();

    REQUIRE(ram.read(LowestRawAddress) == DefaultByte);
    REQUIRE(ram.read(ProgramStartAddress) == DefaultByte);
    REQUIRE(ram.read(HighestRawAddress) == DefaultByte);
}

TEST_CASE("RAM can read and write valid raw memory addresses", "[ram]") {
    Ram ram;

    SECTION("lowest raw memory address") {
        ram.write(LowestRawAddress, FirstTestByte);
        REQUIRE(ram.read(LowestRawAddress) == FirstTestByte);
    }

    SECTION("program start address") {
        ram.write(ProgramStartAddress, SecondTestByte);
        REQUIRE(ram.read(ProgramStartAddress) == SecondTestByte);
    }

    SECTION("highest raw memory address") {
        ram.write(HighestRawAddress, ThirdTestByte);
        REQUIRE(ram.read(HighestRawAddress) == ThirdTestByte);
    }
}

TEST_CASE("RAM rejects invalid raw memory addresses", "[ram]") {
    Ram ram;

    REQUIRE_THROWS_AS(ram.read(InvalidHighAddress), std::runtime_error);
    REQUIRE_THROWS_AS(ram.write(InvalidHighAddress, FirstTestByte), std::runtime_error);

    REQUIRE_THROWS_WITH(
        ram.read(InvalidHighAddress),
        "address value cannot be greater than 4095."
    );
}

TEST_CASE("RAM reads opcodes in big-endian order", "[ram]") {
    Ram ram;

    ram.write(ProgramStartAddress, OpcodeHighByte);
    ram.write(ProgramStartAddress + 1, OpcodeLowByte);

    REQUIRE(ram.readOpcode(ProgramStartAddress) == ExpectedOpcode);
}

TEST_CASE("RAM can read opcode at the last valid opcode start address", "[ram]") {
    Ram ram;

    ram.write(LastOpcodeStartAddress, OpcodeHighByte);
    ram.write(HighestRawAddress, OpcodeLowByte);

    REQUIRE(ram.readOpcode(LastOpcodeStartAddress) == ExpectedOpcode);
}

TEST_CASE("RAM rejects opcode reads that exceed memory boundary", "[ram]") {
    Ram ram;

    REQUIRE_THROWS_AS(ram.readOpcode(HighestRawAddress), std::runtime_error);
    REQUIRE_THROWS_WITH(
        ram.readOpcode(HighestRawAddress),
        "address value cannot be greater than 4095."
    );
}

TEST_CASE("RAM loads ROM into program memory", "[ram]") {
    Ram ram;
    const std::vector<uint8_t> rom { RomByte1, RomByte2, RomByte3, RomByte4 };

    ram.loadROM(rom);

    REQUIRE(ram.read(ProgramStartAddress) == RomByte1);
    REQUIRE(ram.read(ProgramStartAddress + 1) == RomByte2);
    REQUIRE(ram.read(ProgramStartAddress + 2) == RomByte3);
    REQUIRE(ram.read(ProgramStartAddress + 3) == RomByte4);
}

TEST_CASE("RAM loads ROM at a custom valid start address", "[ram]") {
    Ram ram;
    const std::vector<uint8_t> rom { RomByte1, RomByte2 };
    constexpr uint16_t CustomStartAddress = ProgramStartAddress + 0x10;

    ram.loadROM(rom, CustomStartAddress);

    REQUIRE(ram.read(CustomStartAddress) == RomByte1);
    REQUIRE(ram.read(CustomStartAddress + 1) == RomByte2);
}

TEST_CASE("RAM rejects ROM load below program memory", "[ram]") {
    Ram ram;
    const std::vector<uint8_t> rom { RomByte1, RomByte2 };

    REQUIRE_THROWS_AS(ram.loadROM(rom, InvalidProgramAddress), std::runtime_error);
    REQUIRE_THROWS_WITH(
        ram.loadROM(rom, InvalidProgramAddress),
        "startAddress value cannot be lower than 512."
    );
}

TEST_CASE("RAM rejects ROMs that exceed memory boundary", "[ram]") {
    Ram ram;
    const std::vector<uint8_t> rom { RomByte1, RomByte2 };

    REQUIRE_THROWS_AS(ram.loadROM(rom, HighestRawAddress), std::runtime_error);
    REQUIRE_THROWS_WITH(
        ram.loadROM(rom, HighestRawAddress),
        "ROM is exceeding memory boundary."
    );
}
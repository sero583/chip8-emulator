# CHIP-8 Emulator

A CHIP-8 emulator written in modern C++ with Qt for rendering, input handling, and the desktop user interface.

This project started as a personal hobby project with a focus on clean architecture, readable code, and maintainable structure. It also serves as a compact example of emulator architecture, Qt desktop application development, and testable modern C++ project organization.

## Motivation

There are already many CHIP-8 emulators, so the goal here was not to create the most feature-rich implementation. Instead, the focus is on clear separation of responsibilities across CPU, memory, display, input, and application logic.

It is also a fun project because it combines low-level emulation logic with desktop GUI development in a relatively compact codebase.

## Current Status

The emulator is already functional and covers the core parts of CHIP-8 execution, but it is still a work in progress.

Some instructions in the F-family are still missing, and a few opcode behaviors still need refinement for better accuracy and compatibility.

The `0NNN` RCA 1802 call instruction is currently not supported, since it is rarely needed in typical CHIP-8 programs and the project is focused on the most commonly used functionality. It may be added later.

## Features

- CHIP-8 instruction emulation
- ROM loading support
- Display rendering with Qt
- Input handling, including a virtual keypad
- Pause, resume, reset, and step controls
- Modular code structure
- Unit tests for core components
- CMake-based build setup
- Intended to work on Windows, macOS, and Linux

## Tech Stack

- C++20
- Qt 6
- Catch2
- CMake

## Requirements

Before building the project, make sure you have:

- A C++20-compatible compiler
- CMake 3.20 or newer
- Qt 6 with `Core`, `Gui`, `Widgets`, and `Test`
- Catch2 installed so CMake can find it

### Windows note

If you are building on Windows, using vcpkg is recommended because it simplifies dependency management and CMake integration.

If you use vcpkg together with a `vcpkg.json` manifest, dependencies are typically resolved automatically during CMake configuration.

Qt applications may also require deployment of runtime components. If needed, run `windeployqt` on the built executable, for example:

```bash
C:\path\to\vcpkg\installed\x64-windows\tools\qt6\bin\windeployqt.exe C:\path\to\your\build\Debug\CHIP8_Emulator.exe
```

The same may apply to the test executable, depending on your setup.

## Build

Clone the repository:

```bash
git clone https://github.com/sero583/CHIP8_Emulator.git
cd CHIP8_Emulator
```

### Configure with plain CMake

Use this if your dependencies are already installed and discoverable by CMake:

```bash
cmake --fresh -S . -B build
```

If CMake cannot find Qt or Catch2 automatically, provide their install paths through `CMAKE_PREFIX_PATH`:

```bash
cmake --fresh -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt;/path/to/Catch2/install"
```

### Configure with vcpkg

If you use vcpkg, configure the project like this:

```bash
cmake --fresh -S . -B build -DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake"
```

Example:

```bash
cmake --fresh -S . -B build -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Build

Build the project with:

```bash
cmake --build build
```

For Visual Studio or other multi-config generators, use:

```bash
cmake --build build --config Debug
```

### Presets

If the repository provides `CMakePresets.json`, you can also use presets:

```bash
cmake --preset debug
cmake --build --preset debug
```

Note that `cmake --build --preset debug` builds the project. It does not install dependencies by itself.

## Run

Run the emulator from the build output directory.

Examples:

```bash
./build/CHIP8_Emulator
```

or on Windows with a multi-config generator:

```bash
./build/Debug/CHIP8_Emulator.exe
```

## Tests

This project uses Catch2 together with CTest.

Run the tests with:

```bash
ctest --test-dir build --output-on-failure
```

For multi-config generators such as Visual Studio, use:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

If needed, build the test target explicitly first:

```bash
cmake --build build --target CHIP8_Tests --config Debug
```

## Notes

On Windows, Qt applications may require platform plugins at runtime.

If CMake cannot find Qt or Catch2, double-check that:

- both dependencies are installed
- their CMake package files are available
- `CMAKE_PREFIX_PATH` points to the correct locations

## Resources

The following references were useful for learning and verifying CHIP-8 behavior:

- [Tobias V. Langhoff — Write a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Timendus CHIP-8 test suite](https://github.com/Timendus/chip8-test-suite)
- [kripod CHIP-8 ROMs](https://github.com/kripod/chip8-roms)
- [Octo](https://johnearnest.github.io/Octo/)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Created by [Serhat Güler (sero583)](https://github.com/sero583).
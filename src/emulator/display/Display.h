#pragma once

#include <QWidget>

#include "emulator/Emulator.h"

class QPaintEvent;

/**
 * @brief Custom Qt widget for rendering the CHIP-8 display output.
 *
 * This widget acts as the visual representation of the emulator screen.
 * It reads the current display state from the emulator and draws it
 * inside its paint event.
 *
 * The CHIP-8 display is monochrome and has a fixed logical resolution
 * of 64x32 pixels. This widget is responsible for scaling that logical
 * pixel grid to the current widget size.
 */
class Display : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Creates the display widget.
     *
     * Stores a reference to the emulator so the widget can access
     * the current framebuffer when repainting.
     *
     * @param emulatorRef Reference to the emulator instance that owns the display state.
     */
    explicit Display(Emulator& emulatorRef);

    /**
     * @brief Creates the display widget backed by a direct framebuffer reference.
     *
     * Intended for tests so rendering can be verified without exposing mutable
     * framebuffer access through the emulator API.
     *
     * @param testBuffer Framebuffer to render.
     * @param parent Optional parent widget.
     */
    explicit Display(
        const std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>& testBuffer,
        QWidget* parent = nullptr
    );

protected:
    /**
     * @brief Paints the display contents.
     *
     * This function is called by Qt whenever the widget needs to be
     * redrawn. It should render the current CHIP-8 framebuffer by
     * drawing scaled pixels onto the widget.
     *
     * @param event Qt paint event information.
     */
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * @brief Reference to the emulator instance.
     *
     * Used to access the current CHIP-8 display buffer during painting.
     */
    Emulator* emulatorRef = nullptr;
    
    /**
     * @brief Optional framebuffer override used primarily for rendering tests.
     *
     * When set, the widget renders this buffer instead of reading the framebuffer
     * from the emulator instance.
     */
    const std::array<uint8_t, DisplayProperties::CHIP8_DISPLAY_WIDTH * DisplayProperties::CHIP8_DISPLAY_HEIGHT>* bufferOverride = nullptr;
};
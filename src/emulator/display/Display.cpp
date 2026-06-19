#include <QPainter>
#include <QPaintEvent>

#include "emulator/display/Display.h"
#include "emulator/Constants.h"

Display::Display(Emulator& emulatorRef) : QWidget(), emulatorRef(emulatorRef) {}

void Display::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);

    // Background
    painter.fillRect(rect(), Qt::black);

    const auto& buffer = emulatorRef.getDisplayBuffer();

    const int pixelSize = std::min(width() / DisplayProperties::CHIP8_DISPLAY_WIDTH, height() / DisplayProperties::CHIP8_DISPLAY_HEIGHT);

    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    for (int y = 0; y < DisplayProperties::CHIP8_DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DisplayProperties::CHIP8_DISPLAY_WIDTH; ++x) {
            if (buffer[y * DisplayProperties::CHIP8_DISPLAY_WIDTH + x]) {
                painter.drawRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
            }
        }
    }
}
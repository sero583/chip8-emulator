#include <array>

#include <QApplication>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QTest>

#include <catch2/catch_all.hpp>

#include "emulator/Constants.h"
#include "emulator/display/Display.h"
#include "TestUtils.h"

namespace {
    constexpr int DisplayWidth = DisplayProperties::CHIP8_DISPLAY_WIDTH;
    constexpr int DisplayHeight = DisplayProperties::CHIP8_DISPLAY_HEIGHT;
    constexpr int FramebufferSize = DisplayWidth * DisplayHeight;

    constexpr int WidgetWidth = 640;
    constexpr int WidgetHeight = 320;
    constexpr int PixelSize = 10;

    constexpr int FirstPixelX = 0;
    constexpr int FirstPixelY = 0;

    constexpr int SecondPixelX = 1;
    constexpr int SecondPixelY = 1;

    constexpr int OffPixelX = 2;
    constexpr int OffPixelY = 0;

    constexpr QRgb BlackRgb = qRgb(0, 0, 0);
    constexpr QRgb WhiteRgb = qRgb(255, 255, 255);

    int bufferIndex(int x, int y) {
        return y * DisplayWidth + x;
    }

    QPoint pointInsideLogicalPixel(int x, int y) {
        return QPoint(x * PixelSize + PixelSize / 2, y * PixelSize + PixelSize / 2);
    }
}

TEST_CASE("Display renders black background for a cleared framebuffer", "[display]") {
    TestUtils::getApplication();

    std::array<uint8_t, FramebufferSize> buffer{};
    buffer.fill(0);

    Display display(buffer);
    display.resize(WidgetWidth, WidgetHeight);
    display.show();

    REQUIRE(QTest::qWaitForWindowExposed(&display));
    QTest::qWait(50);
    display.update();
    QApplication::processEvents();

    const QImage image = display.grab().toImage();

    REQUIRE_FALSE(image.isNull());
    REQUIRE(image.pixel(pointInsideLogicalPixel(FirstPixelX, FirstPixelY)) == BlackRgb);
    REQUIRE(image.pixel(pointInsideLogicalPixel(SecondPixelX, SecondPixelY)) == BlackRgb);
    REQUIRE(image.pixel(pointInsideLogicalPixel(OffPixelX, OffPixelY)) == BlackRgb);
}

TEST_CASE("Display renders enabled framebuffer pixels as white blocks", "[display]") {
    TestUtils::getApplication();

    std::array<uint8_t, FramebufferSize> buffer{};
    buffer.fill(0);

    buffer[bufferIndex(FirstPixelX, FirstPixelY)] = 1;
    buffer[bufferIndex(SecondPixelX, SecondPixelY)] = 1;

    Display display(buffer);
    display.resize(WidgetWidth, WidgetHeight);
    display.show();

    REQUIRE(QTest::qWaitForWindowExposed(&display));
    QTest::qWait(50);
    display.update();
    QApplication::processEvents();

    const QImage image = display.grab().toImage();

    REQUIRE_FALSE(image.isNull());
    REQUIRE(image.pixel(pointInsideLogicalPixel(FirstPixelX, FirstPixelY)) == WhiteRgb);
    REQUIRE(image.pixel(pointInsideLogicalPixel(SecondPixelX, SecondPixelY)) == WhiteRgb);
    REQUIRE(image.pixel(pointInsideLogicalPixel(OffPixelX, OffPixelY)) == BlackRgb);
}

TEST_CASE("Display renders the last framebuffer pixel in the correct position", "[display]") {
    TestUtils::getApplication();

    std::array<uint8_t, FramebufferSize> buffer{};
    buffer.fill(0);

    constexpr int LastPixelX = DisplayWidth - 1;
    constexpr int LastPixelY = DisplayHeight - 1;

    buffer[bufferIndex(LastPixelX, LastPixelY)] = 1;

    Display display(buffer);
    display.resize(WidgetWidth, WidgetHeight);
    display.show();

    REQUIRE(QTest::qWaitForWindowExposed(&display));
    QTest::qWait(50);
    display.update();
    QApplication::processEvents();

    const QImage image = display.grab().toImage();

    REQUIRE_FALSE(image.isNull());
    REQUIRE(image.pixel(pointInsideLogicalPixel(LastPixelX, LastPixelY)) == WhiteRgb);
    REQUIRE(image.pixel(pointInsideLogicalPixel(LastPixelX - 1, LastPixelY)) == BlackRgb);
}
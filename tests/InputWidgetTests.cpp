#include <cctype>
#include <vector>

#include <QApplication>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <QWidget>

#include <catch2/catch_all.hpp>

#include "emulator/Constants.h"
#include "emulator/Emulator.h"
#include "emulator/input/InputWidget.h"
#include "TestUtils.h"

namespace {
    QPushButton* findButtonByLabel(InputWidget& widget, char label) {
        const auto buttons = widget.findChildren<QPushButton*>();
        const QString labelText = QString(QChar::fromLatin1(label));

        for(QPushButton* button : buttons) {
            if(button!=nullptr && button->text()==labelText) {
                return button;
            }
        }
        return nullptr;
    }

    void flushEvents() {
        QApplication::processEvents();
    }

    void showWidget(QWidget& widget) {
        widget.show();
        flushEvents();
    }

    bool isSupportedTestKey(char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
    }

    Qt::Key toQtKey(char c) {
        switch(std::toupper(static_cast<unsigned char>(c))) {
        case '0': return Qt::Key_0;
        case '1': return Qt::Key_1;
        case '2': return Qt::Key_2;
        case '3': return Qt::Key_3;
        case '4': return Qt::Key_4;
        case '5': return Qt::Key_5;
        case '6': return Qt::Key_6;
        case '7': return Qt::Key_7;
        case '8': return Qt::Key_8;
        case '9': return Qt::Key_9;
        case 'A': return Qt::Key_A;
        case 'B': return Qt::Key_B;
        case 'C': return Qt::Key_C;
        case 'D': return Qt::Key_D;
        case 'E': return Qt::Key_E;
        case 'F': return Qt::Key_F;
        default:  return Qt::Key_Space;
        }
    }

    QString toKeyText(char c) {
        return QString(QChar::fromLatin1(c));
    }

    void sendKeyPress(QWidget& widget, char c) {
        QKeyEvent event(QEvent::KeyPress, toQtKey(c), Qt::NoModifier, toKeyText(c));
        QCoreApplication::sendEvent(&widget, &event);
        flushEvents();
    }

    void sendKeyRelease(QWidget& widget, char c) {
        QKeyEvent event(QEvent::KeyRelease, toQtKey(c), Qt::NoModifier, toKeyText(c));
        QCoreApplication::sendEvent(&widget, &event);
        flushEvents();
    }

    QPushButton* findFirstSupportedButton(InputWidget& widget, char& outLabel) {
        const auto buttons = widget.findChildren<QPushButton*>();

        for(QPushButton* button : buttons) {
            if(button==nullptr || button->text().isEmpty()) {
                continue;
            }

            const char c = button->text().toUpper()[0].toLatin1();
            if(isSupportedTestKey(c)) {
                outLabel = c;
                return button;
            }
        }

        outLabel = '\0';
        return nullptr;
    }

    QPushButton* findFirstLetterButton(InputWidget& widget) {
        const auto buttons = widget.findChildren<QPushButton*>();

        for(QPushButton* button : buttons) {
            if(button!=nullptr && button->text().size()==1 && button->text()[0].isLetter()) {
                return button;
            }
        }

        return nullptr;
    }
}

TEST_CASE("InputWidget smoke test", "[input][smoke]") {
    REQUIRE(true);
}

TEST_CASE("InputWidget builds exactly one button per CHIP-8 key in buttonLayout", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE(buttons.size()==InputProperties::buttonLayout.size());

    for(const auto& pair : InputProperties::buttonLayout) {
        const char label = pair.first;
        const int expectedChip8Key = pair.second;

        QPushButton* button = findButtonByLabel(widget, label);
        REQUIRE(button!=nullptr);
        REQUIRE(button->text()==QString(QChar::fromLatin1(label)));
        REQUIRE(button->property("chip8Key").toInt()==expectedChip8Key);
    }
}

TEST_CASE("InputWidget buttons are enabled by default after construction", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE_FALSE(buttons.empty());

    for(QPushButton* button : buttons) {
        REQUIRE(button->isEnabled());
    }
}

TEST_CASE("InputWidget setButtonsEnabled(false) disables all virtual keypad buttons", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    widget.setButtonsEnabled(false);
    flushEvents();

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE_FALSE(buttons.empty());

    for(QPushButton* button : buttons) {
        REQUIRE_FALSE(button->isEnabled());
    }
}

TEST_CASE("InputWidget setButtonsEnabled(true) re-enables all virtual keypad buttons", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    widget.setButtonsEnabled(false);
    widget.setButtonsEnabled(true);
    flushEvents();

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE_FALSE(buttons.empty());

    for(QPushButton* button : buttons) {
        REQUIRE(button->isEnabled());
    }
}

TEST_CASE("InputWidget creates buttons with expected chip8Key property", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    for(const auto& pair : InputProperties::buttonLayout) {
        QPushButton* button = findButtonByLabel(widget, pair.first);
        REQUIRE(button!=nullptr);
        REQUIRE(button->property("chip8Key").isValid());
        REQUIRE(button->property("chip8Key").toInt()==pair.second);
    }
}

TEST_CASE("Keyboard press on a mapped key sets the matching button down", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);
    widget.setFocus();
    flushEvents();

    char label = '\0';
    QPushButton* target = findFirstSupportedButton(widget, label);

    REQUIRE(target!=nullptr);

    sendKeyPress(widget, label);

    REQUIRE(target->isDown());
}

TEST_CASE("Keyboard release on a mapped key clears the matching button down state", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);
    widget.setFocus();
    flushEvents();

    char label = '\0';
    QPushButton* target = findFirstSupportedButton(widget, label);

    REQUIRE(target!=nullptr);

    sendKeyPress(widget, label);
    REQUIRE(target->isDown());

    sendKeyRelease(widget, label);

    REQUIRE_FALSE(target->isDown());
}

TEST_CASE("Keyboard press on unmapped key does not press any button", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);
    widget.setFocus();
    flushEvents();

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE_FALSE(buttons.empty());

    std::vector<bool> before;
    before.reserve(buttons.size());

    for(QPushButton* button : buttons) {
        before.push_back(button->isDown());
    }

    sendKeyPress(widget, 'Z');

    for(std::size_t i = 0; i < buttons.size(); ++i) {
        REQUIRE(buttons[i]->isDown()==before[i]);
    }
}

TEST_CASE("Keyboard events are ignored when widget is disabled", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);
    widget.setFocus();
    widget.setEnabled(false);
    flushEvents();

    char label = '\0';
    QPushButton* target = findFirstSupportedButton(widget, label);

    REQUIRE(target!=nullptr);
    const bool initialState = target->isDown();

    sendKeyPress(widget, label);

    REQUIRE(target->isDown()==initialState);
}

TEST_CASE("Lowercase keyboard input matches uppercase mapping", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);
    widget.setFocus();
    flushEvents();

    QPushButton* target = findFirstLetterButton(widget);

    if(target==nullptr) {
        SUCCEED();
        return;
    }

    const char upper = target->text()[0].toLatin1();
    const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(upper)));

    sendKeyPress(widget, lower);
    const bool lowerPressed = target->isDown();

    sendKeyRelease(widget, lower);

    sendKeyPress(widget, upper);
    const bool upperPressed = target->isDown();

    REQUIRE(lowerPressed==upperPressed);
}

TEST_CASE("Disabled virtual buttons stay disabled after setButtonsEnabled(false)", "[input]") {
    TestUtils::getApplication();

    Emulator emulator;
    InputWidget widget(emulator);
    showWidget(widget);

    widget.setButtonsEnabled(false);
    flushEvents();

    const auto buttons = widget.findChildren<QPushButton*>();
    REQUIRE_FALSE(buttons.empty());

    for(QPushButton* button : buttons) {
        REQUIRE_FALSE(button->isEnabled());
        REQUIRE_FALSE(button->isDown());
    }
}
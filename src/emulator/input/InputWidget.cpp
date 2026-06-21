#include <QGridLayout>

#include "emulator/Constants.h"
#include "emulator/input/InputWidget.h"

InputWidget::InputWidget(Emulator& emulatorRef, QWidget* parent, bool enableButtons) : emulatorRef(emulatorRef), QWidget(parent) {
    buildWidgets(enableButtons);
}

void InputWidget::setButtonsEnabled(bool enabled) {
    for(auto& entry : buttonsByLabel) {
        // Defensive check
        if(entry.second!=nullptr) {
            entry.second->setEnabled(enabled);
        }
    }
}

void InputWidget::handleKeyEvent(QKeyEvent* event, bool isPressed) {
    if (!isEnabled()) {
        QWidget::keyPressEvent(event);
        return;
    }

    QString text = event->text();
    if (text.isEmpty()) {
        QWidget::keyPressEvent(event);
        return;
    }

    char keyLabel = text.toUpper()[0].toLatin1();

    auto entry = buttonsByLabel.find(keyLabel);
    if (entry == buttonsByLabel.end() || entry->second == nullptr) {
        QWidget::keyPressEvent(event);
        return;
    }

    QPushButton* button = entry->second;
    int chip8Key = button->property("chip8Key").toInt();

    button->setDown(isPressed);
    emulatorRef.setKeyState(chip8Key, isPressed);

    event->accept();
}

void InputWidget::keyPressEvent(QKeyEvent* event) {
    handleKeyEvent(event, true);
}

void InputWidget::keyReleaseEvent(QKeyEvent* event) {
    handleKeyEvent(event, false);
}

void InputWidget::buildWidgets(bool enableButtons) {
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(0);

    for(int i = 0; i < InputProperties::buttonLayout.size(); i++) {
        std::pair<char, int> pair = InputProperties::buttonLayout[i];

        // Need to convert to QString in order to make the char work as a button label
        QPushButton* buttonEach = new QPushButton(QString(QChar::fromLatin1(pair.first)), this);
        buttonEach->setProperty("chip8Key", pair.second);
        buttonEach->setMinimumSize(40, 40);
        buttonEach->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        buttonEach->setEnabled(enableButtons);

        connect(buttonEach, &QPushButton::pressed, this, [this, buttonEach]() {
            int key = buttonEach->property("chip8Key").toInt();
            emulatorRef.setKeyState(key, true);
        });

        connect(buttonEach, &QPushButton::released, this, [this, buttonEach]() {
            int key = buttonEach->property("chip8Key").toInt();
            emulatorRef.setKeyState(key, false);
        });

        buttonsByLabel[pair.first] = buttonEach;

        grid->addWidget(buttonEach, i / InputProperties::rowWidth, i % InputProperties::rowWidth);
    }
    setLayout(grid);
}
#pragma once

#include <array>
#include <unordered_map>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QWidget>

#include "emulator/Emulator.h"

/**
 * @brief Custom Qt widget that handles the CHIP-8 virtual keypad input.
 *
 * The widget receives physical keyboard events directly from Qt as long as it
 * has keyboard focus. It also owns and manages the virtual keypad buttons that
 * are displayed in a grid layout.
 *
 * This keeps the keyboard input logic and keypad UI in one place instead of
 * routing keyboard events through the main window first.
 */
class InputWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Creates the input widget.
     *
     * The widget stores a reference to the emulator instance so it can forward
     * translated CHIP-8 keypad events to the emulator.
     *
     * During construction, the widget can also set up its internal button grid
     * and enable keyboard focus handling so it can receive QKeyEvent press and
     * release events directly from Qt.
     *
     * @param emulatorRef Reference to the emulator instance that will receive translated keypad input events.
     * @param parent Optional parent widget.
     * @param buildWidgets Whether to build the virtual keypad widgets during construction.
     */
    explicit InputWidget(Emulator& emulatorRef, QWidget* parent = nullptr, bool buildWidgets = true);

    /**
     * @brief Enables or disables the virtual keypad buttons. By default buttons are enabled.
     *
     * This can be used to prevent user interaction with the virtual buttons when
     * no ROM is loaded or when the emulator is in a state where input should be
     * ignored. By default, the buttons are enabled, but they can be disabled by passing false to this function. When disabled, the buttons will not respond to user clicks and will appear visually disabled.
     *
     * @param enabled True to enable the buttons, false to disable them.
     */
    void setButtonsEnabled(bool enabled = true);
protected:
    /**
     * @brief Handles physical keyboard key press events.
     *
     * This function is called by Qt when a key is pressed while this widget has
     * keyboard focus. The implementation should translate the Qt key code to the
     * corresponding CHIP-8 keypad key and notify the emulator.
     *
     * @param event The Qt key event containing information about the pressed key.
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief Handles physical keyboard key release events.
     *
     * This function is called by Qt when a key is released while this widget has
     * keyboard focus. The implementation should translate the Qt key code to the
     * corresponding CHIP-8 keypad key and notify the emulator.
     *
     * @param event The Qt key event containing information about the released key.
     */
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    /**
     * @brief Builds the virtual keypad widgets and layout.
     *
     * This function creates the keypad buttons, places them into a grid layout,
     * and connects their signals to the corresponding emulator input handling.
     * 
     * @param enableButtons Whether to enable the buttons after building them. By default, buttons are enabled after being built, but this can be set to false to keep them disabled until explicitly enabled.
     */
    void buildWidgets(bool enableButtons = true);

    /**
     * @brief Handles key events.
     * 
     * @param event The Qt key event to handle.
     * @param isPressed True if the key is being pressed, false if it is being released.
     */
    void handleKeyEvent(QKeyEvent* event, bool isPressed); 

    /**
     * @brief Reference to the emulator instance.
     *
     * Used to forward translated CHIP-8 keypad input to the emulator.
     */
    Emulator& emulatorRef;

    /**
     * @brief Mapping of button labels to their corresponding QPushButton widgets.
     * 
     * Used to access buttons without iterating through the layout, in our scenario for merely mapping keyboard buttons to
     * their corresponding virtual buttons, so that we can simulate button presses.
     */
    std::unordered_map<char, QPushButton*> buttonsByLabel;
};
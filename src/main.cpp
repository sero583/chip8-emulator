#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QGuiApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStyleHints>
#include <Qt>
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "emulator/Constants.h"
#include "emulator/display/Display.h"
#include "emulator/Emulator.h"
#include "emulator/input/InputWidget.h"

namespace {
    /**
     * @brief Creates a placeholder widget with a message prompting the user to load a ROM.
     * 
     * Creates a placeholder widget with a message prompting the user to load a ROM.
     * This is shown in the central area of the main window when no ROM is loaded, and is removed once a ROM is loaded and emulation starts.
     * 
     * @param parent The parent widget for the placeholder.
     * @return A pointer to the created placeholder widget.
     */
    QWidget* createEmulationPlaceholder(QWidget* parent) {
        QWidget* container = new QWidget(parent);

        QVBoxLayout* layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        QLabel* label = new QLabel("Please select a ROM to start emulation", container);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            "background-color: #242424;"
            "color: #9a9a9a;"
            "font-size: 16px;"
        );

        layout->addWidget(label);
        return container;
    }

    /**
     * @brief Creates the display widget and adds it to the provided layout.
     * 
     * @param emulator Reference to the emulator instance, used to link the display to the emulator's display buffer.
     * @param parentLayout The layout to which the display widget should be added. If null, the display will not be added to any layout.
     * @return A pointer to the created display widget. Note that if a parent layout is provided, the display will also be added to that layout, but the caller is still responsible for managing the display's lifetime (e.g. deleting it when no longer needed).
     */
    Display* createDisplay(Emulator& emulator, QBoxLayout* parentLayout) {
        Display* display = new Display(emulator);
        display->setMinimumSize(DisplayProperties::CHIP8_DISPLAY_WIDTH * 10, DisplayProperties::CHIP8_DISPLAY_HEIGHT * 10);

        if(parentLayout!=nullptr) {
            // Add layout to center of parent
            parentLayout->addWidget(display, 0, Qt::AlignmentFlag::AlignCenter);
        }

        return display;
    }

    /**
     * @brief Starts the emulator.
     * 
     * @param cpuTimer The timer controlling the CPU cycles, which will be started if the emulator is not already running.
     * @param resumePauseEmulationAction Optional pointer to the action that toggles emulation state, used to update its text when starting the emulator. If null, no text update will be performed.
     * @param inputPad Optional pointer to the input widget, used to enable its buttons when starting the emulator. If null, no button enabling will be performed.
     * 
     * @return True if the emulator was not already running and is now started, false if it was already running.
     */
    bool startEmulator(QTimer& cpuTimer, QAction* resumePauseEmulationAction = nullptr, InputWidget* inputPad = nullptr) {
        if(!cpuTimer.isActive()) {
            cpuTimer.setTimerType(Qt::PreciseTimer);
            cpuTimer.start(CpuProperties::CPU_TIMER_INTERVAL_MS);

            if(resumePauseEmulationAction!=nullptr) {
                resumePauseEmulationAction->setText("Pause Emulation");
            }

            if(inputPad!=nullptr) {
                inputPad->setButtonsEnabled();
            }
            return true;
        }
        return false;
    }

    /**
     * @brief Stops the emulator.
     * 
     * @param cpuTimer The timer controlling the CPU cycles, which will be stopped if the emulator is running.
     * @param resumePauseEmulationAction Optional pointer to the action that toggles emulation state, used to update its text when stopping the emulator. If null, no text update will be performed.
     * @param inputPad Optional pointer to the input widget, used to disable its buttons when stopping the emulator. If null, no button disabling will be performed.
     * 
     * @return True if the emulator was running and is now stopped, false if it was already stopped.
     */
    bool stopEmulator(QTimer& cpuTimer, QAction* resumePauseEmulationAction = nullptr, InputWidget* inputPad = nullptr) {
        if(cpuTimer.isActive()) {
            cpuTimer.stop();

            if(resumePauseEmulationAction!=nullptr) {
                resumePauseEmulationAction->setText("Resume Emulation");
            }

            if(inputPad!=nullptr) {
                inputPad->setButtonsEnabled(false);
            }
            return true;
        }
        return false;
    }

    /**
     * @brief Toggles the emulator between running and paused states.
     * 
     * @param cpuTimer The timer controlling the CPU cycles, which will be started or stopped based on the current state of the emulator.
     * @param resumePauseEmulationAction Optional pointer to the action that toggles emulation state, used to update its text when toggling the emulator. If null, no text update will be performed.
     * @param inputPad Optional pointer to the input widget, used to enable or disable its buttons based on the new state of the emulator. If null, no button state change will be performed.
     * @param stepEmulationAction Optional pointer to the action that allows stepping through emulation, used to enable or disable it based on the new state of the emulator. If null, no action state change will be performed.
     * 
     * @return True if the emulator is now running, false if it is now paused.
     */
    bool toggleEmulator(QTimer& cpuTimer, QAction* resumePauseEmulationAction = nullptr, InputWidget* inputPad = nullptr, QAction* stepEmulationAction = nullptr) {
        if(cpuTimer.isActive()) {
            if(stepEmulationAction!=nullptr) {
                stepEmulationAction->setDisabled(false);
            }
            return stopEmulator(cpuTimer, resumePauseEmulationAction, inputPad);
        } else {
            if(stepEmulationAction!=nullptr) {
                stepEmulationAction->setDisabled(true);
            }
            return startEmulator(cpuTimer, resumePauseEmulationAction, inputPad);
        }
    }
}

int main(int argc, char* argv[]) {
    // Init QT
    QApplication app(argc, argv);

    // Set application details for settings storage
    QCoreApplication::setOrganizationName("sero583");
    QCoreApplication::setApplicationName("CHIP8_Emulator");

    QSettings settings;

    // Create QT window
    QMainWindow window;
    window.setWindowTitle("CHIP-8 Emulator");
    window.resize(800, 600);


    // Set central content (emulation view or when disabled placeholder text)
    QWidget* centralWidget = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    /*
        Set emulation area. This is done, because when swapping between placeholder and display widget,
        we make sure that it stays on the upper spot where it's supposed to stay at.
    */
    QWidget* emulationArea = new QWidget(centralWidget);
    QVBoxLayout* emulationLayout = new QVBoxLayout(emulationArea);
    emulationLayout->setContentsMargins(0, 0, 0, 0);
    emulationLayout->setSpacing(0);

    // Init Emulator
    Emulator emulator;
    // Display variable
    Display* display = nullptr;
    // Track if ROM has been loaded once, so we only reset when loading a new ROM, not on the first load.
    bool hasLoadedRom = false;

    // Placeholder content before ROM is loaded
    QWidget* emulationPlaceholder = createEmulationPlaceholder(emulationArea);
    emulationLayout->addWidget(emulationPlaceholder);
    layout->addWidget(emulationArea);
    window.setCentralWidget(centralWidget);

    // Create virtual input pad
    InputWidget* inputPad = new InputWidget(emulator, centralWidget, false);
    layout->addWidget(inputPad);
    
    // Setup app menus
    QMenu* fileMenu = window.menuBar()->addMenu("File");
    QMenu* emulationMenu = window.menuBar()->addMenu("Emulation");
    QMenu* appearance = window.menuBar()->addMenu("Theme");
        QMenu* helpMenu = window.menuBar()->addMenu("Help");
    helpMenu->addAction("About", &window, [&window]() {
        QMessageBox aboutBox(&window);
        aboutBox.setWindowTitle("About CHIP-8 Emulator");
        aboutBox.setIcon(QMessageBox::Information);
        aboutBox.setTextFormat(Qt::RichText);
        aboutBox.setStandardButtons(QMessageBox::Ok);
        aboutBox.setText(
            "<h3>CHIP-8 Emulator</h3>"
            "<p>"
            "An open-source hobby project CHIP-8 emulator developed by <b>Serhat Güler (sero583)</b> using C++20 and Qt6."
            "</p>"
            "<p>"
            "<a href=\"https://serhat.gueler.dev\">sero583's Website</a><br>"
            "<a href=\"https://github.com/sero583/chip8-emulator\">GitHub Repository of this Project</a>"
            "</p>"
        );
        aboutBox.exec();
    });
    // Disable emulation menu at startup, since emulation didn't start yet.
    emulationMenu->setDisabled(true);

    // Need to define this one before registering callbacks, since we need to update its text in the callbacks.
    QAction* resumePauseEmulationAction = new QAction("Pause Emulation", &window);

    // Run CPU cycles via QTimer so the GUI stays responsive.
    QTimer cpuTimer(&window);

    QObject::connect(&cpuTimer, &QTimer::timeout, &window, [&]() {
        if (!hasLoadedRom || !cpuTimer.isActive()) {
            return;
        }

        try {
            if(emulator.cycleCpu()) {
                // Visual change requested, update display
                display->update();
            }
        } catch(const std::exception& ex) {
            stopEmulator(cpuTimer, resumePauseEmulationAction, inputPad); // text update is wished, so were fresh for next start.

            // Emulation is no longer in a valid running state
            hasLoadedRom = false;
            emulationMenu->setDisabled(true);
            emulator.reset(false, true);

            // Defensive check: display should exist here, but check anyway for future stop/pause flows.
            if(display!=nullptr) {
                emulationLayout->removeWidget(display);
                display->deleteLater();
                display = nullptr;
            }

            // Show placeholder again when emulation stops due to an error
            if(emulationPlaceholder == nullptr) {
                emulationPlaceholder = createEmulationPlaceholder(emulationArea);
                emulationLayout->addWidget(emulationPlaceholder);
            }

            QMessageBox::critical(&window, "Emulation error", ex.what());
        }
    });

    // Add emulation actions to emulation menu

    /* Note:
        - Load ROM should trigger start emulation anyway
        - Start emulation should just start emulation loop incase it was paused
    */
    emulationMenu->addAction(resumePauseEmulationAction);

    // Create and register step emulation action
    QAction* stepEmulationAction = new QAction("Step Emulation", &window);
    stepEmulationAction->setDisabled(true); // stepping is by default disabled, only on pause it gets enabled.

    emulationMenu->addAction(stepEmulationAction);

    // Register start emulation action
    QObject::connect(resumePauseEmulationAction, &QAction::triggered, &window, [&]() {
        toggleEmulator(cpuTimer, resumePauseEmulationAction, nullptr, stepEmulationAction); // dont disable keypad
    });

    QAction* resetEmulationAction = new QAction("Reset Emulation", &window);
    emulationMenu->addAction(resetEmulationAction);

    // Register start emulation action
    QObject::connect(stepEmulationAction, &QAction::triggered, &window, [&]() {
        if(emulator.cycleCpu()) {
            // Visual change requested, update display
            display->update();
        }
    });

    // Add load ROM action to file menu
    QAction* loadRomAction = new QAction("Load ROM", &window);
    fileMenu->addAction(loadRomAction);

    // Add unload ROM action to file menu
    QAction* unloadRomAction = new QAction("Unload ROM", &window);
    unloadRomAction->setDisabled(true); // Unload is only possible when a ROM is loaded, so disable it by default.
    fileMenu->addAction(unloadRomAction);

    QObject::connect(unloadRomAction, &QAction::triggered, &window, [&]() {
        if(!hasLoadedRom) {
            return;
        }

        stopEmulator(cpuTimer, resumePauseEmulationAction, inputPad);
        emulator.reset(false, true);
        hasLoadedRom = false;
        emulationMenu->setDisabled(true);
        unloadRomAction->setDisabled(true);
        stepEmulationAction->setDisabled(true);

        if(display!=nullptr) {
            emulationLayout->removeWidget(display);
            display->deleteLater();
            display = nullptr;
        }

        if(emulationPlaceholder == nullptr) {
            emulationPlaceholder = createEmulationPlaceholder(emulationArea);
            emulationLayout->addWidget(emulationPlaceholder);
        }
    });

    // Add exit action to file menu
    QAction* exitAction = new QAction("Exit emulator", &window);
    fileMenu->addAction(exitAction);

    QObject::connect(exitAction, &QAction::triggered, &window, [&]() {
        window.close();
    });

    // Register callback function for loading ROM
    QObject::connect(loadRomAction, &QAction::triggered, &window, [&]() {
        QString filePath = QFileDialog::getOpenFileName(
            &window,
            "Load ROM",
            "",
            "ROM Files (*.ch8 *.rom);;All Files (*)"
        );

        if(filePath.isEmpty()) {
            return;
        }

        // Reset before a possibly new ROM whilst one has already been loaded
        if(hasLoadedRom) {
            // Stop CPU timer to stop cycling while we reset and load new ROM
            stopEmulator(cpuTimer); // no text update wished
            emulator.reset(false, true);
            // Set text in emulation menu to pause emulation, as it will be running from now on even if it was paused before
            resumePauseEmulationAction->setText("Pause Emulation");
        }

        if(!emulator.loadROMFromFile(filePath)) {
            QMessageBox::warning(&window, "Error", "ROM couldn't be loaded.");
        } else {
            // If it was loaded in this case for first time, we need to remove placeholder and init rest.
            if(!hasLoadedRom) {
                emulationLayout->removeWidget(emulationPlaceholder);
                emulationPlaceholder->deleteLater();
                emulationPlaceholder = nullptr;
                // Init display with reference to emulator's display buffer
                display = createDisplay(emulator, emulationLayout);
                // Enable emulation menu
                emulationMenu->setDisabled(false);
            }
            startEmulator(cpuTimer, resumePauseEmulationAction, inputPad);
            hasLoadedRom = true;
            unloadRomAction->setDisabled(false);
        }
    });

    // Register reset emulation action
    QObject::connect(resetEmulationAction, &QAction::triggered, &window, [&]() {
        /*
            Not adding any checks whether running nor loaded, since widget will be disabled simply in those scenarios
            and were expecting to have an already correct state.
        */
       
        // Stop cycling CPU
        display->update(); // Update display to reflect reset state
        stopEmulator(cpuTimer); // dont pass action button since we don't want to change the resume/pause text in the menu when resetting, as it doesn't affect the paused/running state
        // Reset emulator state with restoring ROM
        emulator.reset(true);
        // Restart CPU timer to resume emulation
        startEmulator(cpuTimer, resumePauseEmulationAction); // same reason as above
        // Stepping is only allowed when paused, so disable it when resetting since after reset emulation always runs
        stepEmulationAction->setDisabled(true);

        // Show user message box about reset
        QMessageBox::information(&window, "Emulator Reset", "Emulator has been reset. ROM has been restored and emulation resumed.");
    });

    // Theme menu with system, light and dark mode options
    QAction* systemThemeAction = new QAction("System Theme", &window);
    QAction* lightThemeAction = new QAction("Light Theme", &window);
    QAction* darkThemeAction = new QAction("Dark Theme", &window);

    systemThemeAction->setCheckable(true);
    lightThemeAction->setCheckable(true);
    darkThemeAction->setCheckable(true);

    QActionGroup* themeGroup = new QActionGroup(&window);
    themeGroup->setExclusive(true);
    themeGroup->addAction(systemThemeAction);
    themeGroup->addAction(lightThemeAction);
    themeGroup->addAction(darkThemeAction);

    appearance->addAction(systemThemeAction);
    appearance->addAction(lightThemeAction);
    appearance->addAction(darkThemeAction);

    auto* styleHints = QGuiApplication::styleHints();

    auto applyThemeMode = [&](const QString& mode) {
        if (mode == "light") {
            styleHints->setColorScheme(Qt::ColorScheme::Light);
        } else if (mode == "dark") {
            styleHints->setColorScheme(Qt::ColorScheme::Dark);
        } else {
            styleHints->setColorScheme(Qt::ColorScheme::Unknown);
        }
    };

    auto syncThemeActions = [&](const QString& mode) {
        systemThemeAction->setChecked(mode == "system");
        lightThemeAction->setChecked(mode == "light");
        darkThemeAction->setChecked(mode == "dark");
    };

    QObject::connect(systemThemeAction, &QAction::triggered, &window, [&]() {
        const QString mode = "system";
        settings.setValue("appearance/themeMode", mode);
        applyThemeMode(mode);
        syncThemeActions(mode);
    });

    QObject::connect(lightThemeAction, &QAction::triggered, &window, [&]() {
        const QString mode = "light";
        settings.setValue("appearance/themeMode", mode);
        applyThemeMode(mode);
        syncThemeActions(mode);
    });

    QObject::connect(darkThemeAction, &QAction::triggered, &window, [&]() {
        const QString mode = "dark";
        settings.setValue("appearance/themeMode", mode);
        applyThemeMode(mode);
        syncThemeActions(mode);
    });

    QObject::connect(
        styleHints,
        &QStyleHints::colorSchemeChanged,
        &window,
        [&](Qt::ColorScheme) {
            const QString savedMode = settings.value("appearance/themeMode", "system").toString();
            if (savedMode == "system") {
                syncThemeActions(savedMode);
            }
        }
    );

    // Restore saved theme on startup
    const QString savedThemeMode = settings.value("appearance/themeMode", "system").toString();

    syncThemeActions(savedThemeMode);
    applyThemeMode(savedThemeMode);
    window.show();

    return app.exec();
}
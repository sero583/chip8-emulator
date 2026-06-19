#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>
#include <Qt>
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "emulator/Constants.h"
#include "emulator/display/Display.h"
#include "emulator/Emulator.h"

namespace {
    /**
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
     * Creates the display widget and adds it to the provided layout.
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
}

int main(int argc, char *argv[]) {
    // Init QT
    QApplication app(argc, argv);

    // Create QT window
    QMainWindow window;
    window.setWindowTitle("CHIP-8 Emulator");
    window.resize(800, 600);

    // Set central content (emulation view or when disabled placeholder text)
    QWidget *centralWidget = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Placeholder content before ROM is loaded
    QWidget *emulationPlaceholder = createEmulationPlaceholder(centralWidget);
    layout->addWidget(emulationPlaceholder);
    window.setCentralWidget(centralWidget);

    // Init Emulator
    Emulator emulator;
    // Display variable
    Display *display = nullptr;
    // Track if ROM has been loaded once, so we only reset when loading a new ROM, not on the first load.
    bool hasLoadedRom = false;
    
    // Setup app menus
    QMenu *fileMenu = window.menuBar()->addMenu("File");
    QMenu *emulationMenu = window.menuBar()->addMenu("Emulation");
    // Disable emulation menu at startup, since emulation didn't start yet.
    emulationMenu->setDisabled(true);

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
        } catch (const std::exception& ex) {
            emulationMenu->setDisabled(true);
            QMessageBox::critical(&window, "Emulation error", ex.what());

            // Show placeholder again when emulation stops due to an error
            emulationPlaceholder = createEmulationPlaceholder(&window);
            layout->addWidget(emulationPlaceholder);

            /*
                Actually impossible for display to be null here since the only way to stop emulation is via an error as of now
                after a ROM has been loaded, but we might want to add a manual stop/pause button in the future that doesn't reset the emulator state,
                so we should be defensive here and only remove the display when it's not null, just in case we reuse this code
            */
            if(display!=nullptr) {
                layout->removeWidget(display);
                delete display;
                display = nullptr;
            }
        }
    });

    // Add emulation actions to emulation menu

    /* Note:
        - Load ROM should trigger start emulation anyway
        - Start emulation should just start emulation loop incase it was paused
    */
    QAction *resumePauseEmulationAction = new QAction("Pause Emulation", &window);
    emulationMenu->addAction(resumePauseEmulationAction);

    // Register start emulation action
    QObject::connect(resumePauseEmulationAction, &QAction::triggered, &window, [&]() {
        // Defensive check. Should be impossible to be triggered without a loaded ROM or already running cpuTimer.
        if(!cpuTimer.isActive()) {
            // Schedule timer to start
            cpuTimer.setTimerType(Qt::PreciseTimer); // not sure if this is necessary, since it has been set previously, but wont bite neither
            cpuTimer.start(1);
            resumePauseEmulationAction->setText("Pause Emulation");
        } else {
            cpuTimer.stop();
            resumePauseEmulationAction->setText("Resume Emulation");
        }
    });

    QAction *resetEmulationAction = new QAction("Reset Emulation", &window);
    emulationMenu->addAction(resetEmulationAction);

    QAction *stepEmulationAction = new QAction("Step Emulation", &window);
    emulationMenu->addAction(stepEmulationAction);

    // Register start emulation action
    QObject::connect(stepEmulationAction, &QAction::triggered, &window, [&]() {
        /*
            Not adding any checks whether running nor loaded, since widget will be disabled simply in those scenarios
            and were expecting to have an already correct state.
        */

        if(emulator.cycleCpu()) {
            // Visual change requested, update display
            display->update();
        }
    });


    // Add load ROM action to file menu
    QAction *loadRomAction = new QAction("Load ROM", &window);
    fileMenu->addAction(loadRomAction);

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
            cpuTimer.stop();
            emulator.reset(false, true);
            // Set text in emulation menu to pause emulation, as it will be running from now on even if it was paused before
            resumePauseEmulationAction->setText("Pause Emulation");
        }

        if(!emulator.loadROMFromFile(filePath)) {
            QMessageBox::warning(&window, "Error", "ROM couldn't be loaded.");
        } else {
            // If it was loaded in this case for first time, we need to remove placeholder and init rest.
            if(!hasLoadedRom) {
                layout->removeWidget(emulationPlaceholder);
                delete emulationPlaceholder;
                // Init display with reference to emulator's display buffer
                display = createDisplay(emulator, layout);
                // Enable emulation menu
                emulationMenu->setDisabled(false);
            }
            
            // Start the CPU timer; actual firing frequency depends on the event loop and OS scheduling.
            cpuTimer.setTimerType(Qt::PreciseTimer);
            cpuTimer.start(1);

            hasLoadedRom = true;
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
        cpuTimer.stop();
        // Reset emulator state with restoring ROM
        emulator.reset(true);
        // Restart CPU timer to resume emulation
        cpuTimer.setTimerType(Qt::PreciseTimer); // not sure if this is necessary, since it has been set previously, but wont bite neither
        cpuTimer.start(1);


        // Show user message box about reset
        QMessageBox::information(&window, "Emulator Reset", "Emulator has been reset. ROM has been restored and emulation resumed.");
    });


    window.show();
    return app.exec();
}
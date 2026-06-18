#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QTimer>

#include "emulator/Emulator.h"

int main(int argc, char *argv[]) {
    // Init QT
    QApplication app(argc, argv);

    // Create QT window
    QMainWindow window;
    window.setWindowTitle("CHIP-8 Emulator");
    window.resize(800, 600);

    // Init Emulator
    Emulator emulator;
    // Track if ROM has been loaded once, so we only reset when loading a new ROM, not on the first load.
    bool hasLoadedRom = false;
    // Track if emulation loop should be running, so we can pause and resume it.
    bool isRunning = false;

    // Run CPU cycles via QTimer so the GUI stays responsive.
    QTimer cpuTimer(&window);

    QObject::connect(&cpuTimer, &QTimer::timeout, &window, [&]() {
        if (!hasLoadedRom || !isRunning) {
            return;
        }

        try {
            emulator.cycleCpu();
        } catch (const std::exception& ex) {
            isRunning = false;
            QMessageBox::critical(&window, "Emulation error", ex.what());
        }
    });
    
    // Setup app menus
    QMenu *fileMenu = window.menuBar()->addMenu("File");
    QMenu *emulationMenu = window.menuBar()->addMenu("Emulation");

    // Add emulation actions to emulation menu
    // TODO: These actions don't do anything yet, but they should eventually trigger the appropriate functionality in the emulator.
    
    /* Note:
        - Load ROM should trigger start emulation anyway
        - Start emulation should just start emulation loop incase it was paused
    */
    QAction *startEmulationAction = new QAction("Start Emulation", &window);
    emulationMenu->addAction(startEmulationAction);
    QAction *pauseEmulationAction = new QAction("Pause Emulation", &window);
    emulationMenu->addAction(pauseEmulationAction);
    QAction *resetEmulationAction = new QAction("Reset Emulation", &window);
    emulationMenu->addAction(resetEmulationAction);
    QAction *stepEmulationAction = new QAction("Step Emulation", &window);
    emulationMenu->addAction(stepEmulationAction);

    // Add load ROM action to file menu
    QAction *loadRomAction = new QAction("Load ROM", &window);
    fileMenu->addAction(loadRomAction);

    // Register callback function
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
            isRunning = false;
            emulator.reset();
        }

        if(!emulator.loadROMFromFile(filePath)) {
            QMessageBox::warning(&window, "Error", "ROM couldn't be loaded.");
        } else {
            hasLoadedRom = true;
            isRunning = true;
        }
    });

    // Start the CPU timer; actual firing frequency depends on the event loop and OS scheduling.
    cpuTimer.setTimerType(Qt::PreciseTimer);
    cpuTimer.start(1);

    window.show();
    return app.exec();
}
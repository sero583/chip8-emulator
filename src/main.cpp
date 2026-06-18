#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "emulator/Emulator.h"

namespace {
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

            // Show placeholder again when emulation stops due to an error
            emulationPlaceholder = createEmulationPlaceholder(&window);
            layout->addWidget(emulationPlaceholder);
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

            // Remove placeholder label when ROM is loaded
            layout->removeWidget(emulationPlaceholder);
            delete emulationPlaceholder;
        }
    });

    // Start the CPU timer; actual firing frequency depends on the event loop and OS scheduling.
    cpuTimer.setTimerType(Qt::PreciseTimer);
    cpuTimer.start(1);

    window.show();
    return app.exec();
}
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

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
    
    // Setup app menu
    QMenu *fileMenu = window.menuBar()->addMenu("File");
    
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
            emulator.reset();
        }

        if(!emulator.loadROMFromFile(filePath)) {
            QMessageBox::warning(&window, "Error", "ROM couldn't be loaded.");
        } else {
            hasLoadedRom = true;    
        }
    });

    
    window.show();
    return app.exec();
}
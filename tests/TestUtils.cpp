#include "TestUtils.h"

#include <stdexcept>

#include <QApplication>
#include <QByteArray>

namespace TestUtils {
    QApplication& getApplication(bool headless, bool debugPlugins) {
        static bool configured = false;
        static bool configuredHeadless = false;
        static bool configuredDebugPlugins = false;

        if(!configured) {
            configured = true;
            configuredHeadless = headless;
            configuredDebugPlugins = debugPlugins;

            if(debugPlugins) {
                qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
            }

            if(headless) {
                qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
            }
        } else {
            if(configuredHeadless!=headless || configuredDebugPlugins!=debugPlugins) {
                throw std::logic_error(
                    "TestUtils::getApplication() was called with different configuration after QApplication was already initialized."
                );
            }
        }

        static int argc = 1;
        static char appName[] = "CHIP8_Tests";
        static char* argv[] = { appName, nullptr };
        static QApplication app(argc, argv);

        return app;
    }
}
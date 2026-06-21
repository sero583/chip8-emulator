#pragma once

class QApplication;

/**
 * @brief Shared utilities for automated tests.
 *
 * This namespace contains helper functions used across the test suite,
 * such as initializing and retrieving the shared Qt application instance.
 */
namespace TestUtils {
    /**
     * @brief Gets the shared QApplication instance for GUI-related tests.
     *
     * Qt widgets require a QApplication to exist before they can be created
     * or shown. This helper lazily creates a single shared instance and
     * returns it for reuse across all tests in the same process.
     *
     * If headless mode is requested, the corresponding Qt platform environment
     * variables are configured before the application instance is created.
     * The first call decides the mode for the lifetime of the process.
     *
     * @param headless When true, requests headless Qt platform setup before creating QApplication.
     * @param debugPlugins When true, enables Qt plugin loading diagnostics before creating QApplication.
     * 
     * @return Reference to the shared QApplication instance.
     */
    QApplication& getApplication(bool headless = true, bool debugPlugins = false);
}
/**
 * @file test_module.h
 * @brief Interface voor testmodules.
 *
 * Elke testmodule (SD-kaart, modem, GPS, sensoren) implementeert deze interface.
 * main.cpp gebruikt dit om een uniform menu en test runner te bieden.
 */

#pragma once

#include <Arduino.h>

/**
 * Testmodule structuur.
 * Elke module registreert zichzelf met een naam en twee functies:
 *   - setup(): initialiseer hardware voor deze test
 *   - run():   voer de test uit, return true als geslaagd
 */
struct TestModule {
    const char* name;           // Weergavenaam in het menu
    const char* description;    // Korte beschrijving
    bool (*setup)();            // Initialisatie (return false = overslaan)
    bool (*run)();              // Test uitvoeren
    bool enabled;               // Beschikbaar op dit board? (ingesteld via hardware.h)
};

// ── Hulpfuncties voor serial output (gedeeld door alle modules) ─────────────

inline void printHeader(const char* title) {
    Serial.println();
    Serial.println("────────────────────────────────────────");
    Serial.println(title);
    Serial.println("────────────────────────────────────────");
}

inline void printSeparator() {
    Serial.println("========================================");
}

inline void printPass(const char* name) {
    Serial.printf("  [PASS] %s\n", name);
}

inline void printFail(const char* name) {
    Serial.printf("  [FAIL] %s\n", name);
}

inline void printSkip(const char* name) {
    Serial.printf("  [SKIP] %s (niet beschikbaar op dit board)\n", name);
}

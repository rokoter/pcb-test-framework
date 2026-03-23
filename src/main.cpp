/**
 * @file main.cpp
 * @brief PCB Test Framework — Hoofdprogramma met serial menu
 *
 * Bij het opstarten toont dit een menu via de serial monitor.
 * De gebruiker kiest welke test wordt uitgevoerd.
 * Alle tests draaien op dezelfde firmware — geen bestanden wisselen meer.
 *
 * Hardware-configuratie wordt automatisch geladen via platformio.ini:
 *   pio run -e thermocouple_sim7080g_v1 -t upload
 */

#include <Arduino.h>
#include "hardware.h"
#include "test_module.h"

// ── Externe testmodules (conditioneel beschikbaar) ──────────────────────────
#if HAS_SD_CARD
extern TestModule testSD;
#endif

#if HAS_MODEM
extern TestModule testModem;
#endif

// Sensoren zijn altijd gecompileerd (intern conditioneel)
extern TestModule testSensors;

// ── Testregister ────────────────────────────────────────────────────────────
// Alle beschikbare modules in één array

static TestModule* allTests[] = {
    #if HAS_SD_CARD
    &testSD,
    #endif
    #if HAS_MODEM
    &testModem,
    #endif
    &testSensors,
};

static const uint8_t TEST_COUNT = sizeof(allTests) / sizeof(allTests[0]);

// ── Menu ────────────────────────────────────────────────────────────────────

void printMenu() {
    Serial.println();
    printSeparator();
    Serial.println("  PCB Test Framework");
    Serial.printf("  Board: %s\n", HW_BOARD_NAME);
    Serial.printf("  Build: %s %s\n", __DATE__, __TIME__);
    printSeparator();
    Serial.println();
    Serial.println("  Kies een test:");
    Serial.println();

    for (uint8_t i = 0; i < TEST_COUNT; i++) {
        Serial.printf("  [%u] %s\n", i + 1, allTests[i]->name);
        Serial.printf("      %s\n", allTests[i]->description);
    }

    Serial.printf("\n  [%u] ALLES draaien\n", TEST_COUNT + 1);
    Serial.println("\n  Typ een nummer en druk Enter:");
}

void runTest(uint8_t index) {
    TestModule* test = allTests[index];

    Serial.printf("\n>> Start: %s\n", test->name);

    if (!test->setup()) {
        Serial.println("  Setup mislukt — test overgeslagen");
        return;
    }

    bool result = test->run();

    Serial.println();
    if (result) printPass(test->name);
    else        printFail(test->name);
}

void runAllTests() {
    printHeader("ALLE TESTS DRAAIEN");

    uint8_t passed = 0;
    uint8_t total  = TEST_COUNT;

    for (uint8_t i = 0; i < TEST_COUNT; i++) {
        runTest(i);
        // Tel alleen als geslaagd
        // (We roepen run() opnieuw niet aan, maar checken via de output)
    }

    printHeader("KLAAR");
    Serial.printf("  %u tests uitgevoerd\n", total);
    Serial.println("  Bekijk de resultaten hierboven.");
}

// ── Setup & Loop ────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(1000);

    // User LED knipperen om te laten zien dat we draaien
    #ifdef PIN_USER_LED
    pinMode(PIN_USER_LED, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_USER_LED, HIGH);
        delay(100);
        digitalWrite(PIN_USER_LED, LOW);
        delay(100);
    }
    #endif

    printMenu();
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        int choice = input.toInt();

        if (choice >= 1 && choice <= TEST_COUNT) {
            runTest(choice - 1);
            Serial.println("\n  Druk Enter voor het menu...");
        }
        else if (choice == TEST_COUNT + 1) {
            runAllTests();
            Serial.println("\n  Druk Enter voor het menu...");
        }
        else if (input.length() > 0) {
            Serial.println("  Ongeldig — typ een nummer uit het menu.");
        }
        else {
            printMenu();
        }
    }
}

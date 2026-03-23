/**
 * @file test_modem.cpp
 * @brief Testmodule: LTE Modem (SIM7080G / SIM7070G)
 *
 * Werkt met beide modem-types. Verschil wordt via hardware.h en build flags
 * afgehandeld (bijv. RTS/CTS beschikbaarheid, GNSS beperkingen).
 */

#include "hardware.h"
#include "test_module.h"

#if HAS_MODEM

#define MODEM_SERIAL  Serial1
#define AT_TIMEOUT_MS 3000
#define AT_RETRIES    5

// ── AT hulpfuncties ─────────────────────────────────────────────────────────

static bool sendAT(const char* cmd, const char* expected = "OK", uint32_t timeout = AT_TIMEOUT_MS) {
    while (MODEM_SERIAL.available()) MODEM_SERIAL.read();

    Serial.printf("  >> %s\n", cmd);
    MODEM_SERIAL.println(cmd);

    String response = "";
    uint32_t start = millis();

    while (millis() - start < timeout) {
        while (MODEM_SERIAL.available()) {
            response += (char)MODEM_SERIAL.read();
        }
        if (response.indexOf(expected) >= 0) {
            response.trim();
            Serial.printf("  << %s\n", response.c_str());
            return true;
        }
        if (response.indexOf("ERROR") >= 0) {
            response.trim();
            Serial.printf("  << ERROR: %s\n", response.c_str());
            return false;
        }
    }

    response.trim();
    Serial.printf("  << TIMEOUT (\"%s\")\n", response.c_str());
    return false;
}

static bool sendATRetry(const char* cmd, const char* expected = "OK") {
    for (uint8_t i = 0; i < AT_RETRIES; i++) {
        if (i > 0) {
            Serial.printf("  [poging %u/%u]\n", i + 1, AT_RETRIES);
            delay(500);
        }
        if (sendAT(cmd, expected)) return true;
    }
    return false;
}

// ── Power-on sequentie ──────────────────────────────────────────────────────

static bool modemPowerOn() {
    Serial.println("\n  Modem inschakelen...");

    #ifdef PIN_SIM_STATUS
    if (digitalRead(PIN_SIM_STATUS) == HIGH) {
        Serial.println("  STATUS al HIGH — modem is al actief");
        return true;
    }
    #endif

    digitalWrite(PIN_SIM_PWR, HIGH);
    delay(1200);
    digitalWrite(PIN_SIM_PWR, LOW);

    #ifdef PIN_SIM_STATUS
    Serial.print("  Wachten op STATUS HIGH");
    uint32_t start = millis();
    while (millis() - start < 10000) {
        if (digitalRead(PIN_SIM_STATUS) == HIGH) {
            Serial.printf(" OK (%lu ms)\n", millis() - start);
            return true;
        }
        Serial.print(".");
        delay(250);
    }
    Serial.println(" TIMEOUT");
    return false;
    #else
    // Geen STATUS pin — wacht vast
    Serial.println("  Geen STATUS pin — wacht 5s...");
    delay(5000);
    return true;
    #endif
}

// ── Module interface ────────────────────────────────────────────────────────

static bool modemSetup() {
    // Pin configuratie
    pinMode(PIN_SIM_PWR, OUTPUT);
    digitalWrite(PIN_SIM_PWR, LOW);

    #ifdef PIN_SIM_STATUS
    pinMode(PIN_SIM_STATUS, INPUT);
    #endif

    #if MODEM_HAS_RTS_CTS
    pinMode(PIN_SIM_DTR, OUTPUT);
    pinMode(PIN_SIM_RTS, OUTPUT);
    pinMode(PIN_SIM_CTS, INPUT);
    pinMode(PIN_SIM_RI,  INPUT);
    digitalWrite(PIN_SIM_DTR, LOW);
    digitalWrite(PIN_SIM_RTS, LOW);
    #endif

    // UART starten
    MODEM_SERIAL.begin(MODEM_BAUD, SERIAL_8N1, PIN_SIM_RXD, PIN_SIM_TXD);
    return true;
}

static bool modemRun() {
    printHeader("MODEM TEST");

    #if defined(MODEM_TYPE_SIM7080G)
    Serial.println("  Modem type: SIM7080G");
    #elif defined(MODEM_TYPE_SIM7070G)
    Serial.println("  Modem type: SIM7070G");
    Serial.println("  Let op: GNSS en cellular niet gelijktijdig!");
    #endif

    Serial.printf("  UART: TX=IO%d  RX=IO%d  @ %d baud\n",
                  PIN_SIM_TXD, PIN_SIM_RXD, MODEM_BAUD);
    #if MODEM_HAS_RTS_CTS
    Serial.printf("  Flow control: RTS=IO%d  CTS=IO%d\n", PIN_SIM_RTS, PIN_SIM_CTS);
    #else
    Serial.println("  Flow control: geen");
    #endif

    // Testsequentie
    struct { const char* naam; bool ok; } tests[6];
    uint8_t idx = 0;

    tests[idx++] = { "Power-on",         modemPowerOn() };
    delay(2000);

    tests[idx++] = { "AT communicatie",   sendATRetry("AT") };
    tests[idx++] = { "Echo uit",          sendAT("ATE0") };
    tests[idx++] = { "Modem info",        sendAT("ATI") };

    Serial.println("\n  SIM-kaart check:");
    tests[idx++] = { "SIM-kaart",         sendAT("AT+CPIN?", "CPIN") };

    Serial.println("\n  Signaalsterkte:");
    tests[idx++] = { "Netwerk",           sendAT("AT+CSQ") };

    // Samenvatting
    printHeader("MODEM RESULTATEN");
    uint8_t passed = 0;
    for (uint8_t i = 0; i < idx; i++) {
        if (tests[i].ok) { printPass(tests[i].naam); passed++; }
        else             { printFail(tests[i].naam); }
    }
    Serial.printf("\n  %u/%u tests geslaagd\n", passed, idx);

    return passed >= 3;  // Power-on + AT + echo = minimaal werkend
}

// ── Registratie ─────────────────────────────────────────────────────────────
TestModule testModem = {
    .name        = "LTE Modem",
    .description = "Power-on, AT communicatie, SIM-kaart, signaal",
    .setup       = modemSetup,
    .run         = modemRun,
    .enabled     = true
};

#endif // HAS_MODEM

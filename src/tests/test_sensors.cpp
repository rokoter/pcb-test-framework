/**
 * @file test_sensors.cpp
 * @brief Testmodule: Sensoren (Thermocouples, Fuel Gauge, Charge Controller)
 *
 * Conditioneel gecompileerd: alleen de sensoren die op het board zitten
 * worden getest (via BOARD_HAS_* flags in platformio.ini).
 */

#include "hardware.h"
#include "test_module.h"
#include <Wire.h>

#if HAS_THERMOCOUPLE
#include <SPI.h>
#include <Adafruit_MAX31856.h>
static SPIClass spiTC(FSPI);
#endif

// ── I2C hulpfuncties ────────────────────────────────────────────────────────

static bool i2cRead16(TwoWire& bus, uint8_t addr, uint8_t reg, uint16_t& value) {
    bus.beginTransmission(addr);
    bus.write(reg);
    if (bus.endTransmission(false) != 0) return false;
    if (bus.requestFrom((uint8_t)addr, (uint8_t)2) != 2) return false;
    uint8_t lo = bus.read();
    uint8_t hi = bus.read();
    value = ((uint16_t)hi << 8) | lo;
    return true;
}

static bool i2cRead8(TwoWire& bus, uint8_t addr, uint8_t reg, uint8_t& value) {
    bus.beginTransmission(addr);
    bus.write(reg);
    if (bus.endTransmission(false) != 0) return false;
    if (bus.requestFrom((uint8_t)addr, (uint8_t)1) != 1) return false;
    value = bus.read();
    return true;
}

// ── Thermocouple test ───────────────────────────────────────────────────────

#if HAS_THERMOCOUPLE
static bool testThermocouple(Adafruit_MAX31856& tc, const char* label) {
    Serial.printf("  [%s] ", label);

    if (!tc.begin()) {
        Serial.println("FAIL: begin() mislukt");
        return false;
    }

    tc.setThermocoupleType(MAX31856_TCTYPE_K);
    float cjTemp = tc.readCJTemperature();
    float tcTemp = tc.readThermocoupleTemperature();
    uint8_t fault = tc.readFault();

    Serial.printf("TC=%.1f C  CJ=%.1f C", tcTemp, cjTemp);

    if (fault & MAX31856_FAULT_OPEN) Serial.print("  [OPEN]");
    if (fault == 0)                  Serial.print("  [OK]");
    Serial.println();

    return true;  // SPI communicatie werkt, OPEN is verwacht zonder TC
}
#endif

// ── Fuel Gauge test ─────────────────────────────────────────────────────────

#if HAS_FUEL_GAUGE
static bool testFuelGauge() {
    Wire.beginTransmission(I2C1_FUEL_GAUGE_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.printf("  FAIL: geen ACK op 0x%02X\n", I2C1_FUEL_GAUGE_ADDR);
        return false;
    }
    Serial.printf("  OK: ACK op 0x%02X\n", I2C1_FUEL_GAUGE_ADDR);

    uint16_t voltage = 0;
    if (i2cRead16(Wire, I2C1_FUEL_GAUGE_ADDR, 0x04, voltage)) {
        Serial.printf("  Spanning: %u mV\n", voltage);
    }

    uint16_t soc = 0;
    if (i2cRead16(Wire, I2C1_FUEL_GAUGE_ADDR, 0x1C, soc)) {
        Serial.printf("  SOC: %u %%\n", soc);
    }

    return true;
}
#endif

// ── Charge Controller test ──────────────────────────────────────────────────

#if HAS_CHARGE_CONTROLLER
static bool testChargeController() {
    Wire1.beginTransmission(I2C2_CHARGE_ADDR);
    if (Wire1.endTransmission() != 0) {
        Serial.printf("  FAIL: geen ACK op 0x%02X\n", I2C2_CHARGE_ADDR);
        return false;
    }
    Serial.printf("  OK: ACK op 0x%02X\n", I2C2_CHARGE_ADDR);

    uint8_t status = 0;
    if (i2cRead8(Wire1, I2C2_CHARGE_ADDR, 0x08, status)) {
        uint8_t chrgStat = (status >> 4) & 0x03;
        const char* labels[] = { "Niet laden", "Pre-charge", "Snel laden", "Klaar" };
        Serial.printf("  Laadstatus: %s\n", labels[chrgStat]);
    }

    uint8_t fault = 0;
    if (i2cRead8(Wire1, I2C2_CHARGE_ADDR, 0x09, fault)) {
        Serial.printf("  Fouten: %s\n", fault == 0 ? "geen" : "aanwezig!");
    }

    return true;
}
#endif

// ── Module interface ────────────────────────────────────────────────────────

static bool sensorsSetup() {
    #if HAS_THERMOCOUPLE
    pinMode(PIN_SENSORS_PWR, OUTPUT);
    digitalWrite(PIN_SENSORS_PWR, HIGH);
    delay(50);
    spiTC.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);
    #endif

    #if HAS_FUEL_GAUGE
    Wire.begin(PIN_I2C1_SDA, PIN_I2C1_SCL);
    #endif

    #if HAS_CHARGE_CONTROLLER
    Wire1.begin(PIN_I2C2_SDA, PIN_I2C2_SCL);
    #endif

    return true;
}

static bool sensorsRun() {
    printHeader("SENSOR TEST");
    bool allOk = true;

    #if HAS_THERMOCOUPLE
    Serial.println("\n  === Thermocouples (MAX31856) ===");
    Adafruit_MAX31856 tc1(PIN_TC1_CS, &spiTC);
    allOk &= testThermocouple(tc1, "TC1");
    #if TC_COUNT >= 2
    Adafruit_MAX31856 tc2(PIN_TC2_CS, &spiTC);
    allOk &= testThermocouple(tc2, "TC2");
    #endif
    #if TC_COUNT >= 3
    Adafruit_MAX31856 tc3(PIN_TC3_CS, &spiTC);
    allOk &= testThermocouple(tc3, "TC3");
    #endif
    #endif

    #if HAS_FUEL_GAUGE
    Serial.println("\n  === Fuel Gauge (BQ27220) ===");
    allOk &= testFuelGauge();
    #endif

    #if HAS_CHARGE_CONTROLLER
    Serial.println("\n  === Charge Controller (MP2731) ===");
    allOk &= testChargeController();
    #endif

    return allOk;
}

TestModule testSensors = {
    .name        = "Sensoren",
    .description = "Thermocouples, Fuel Gauge, Charge Controller",
    .setup       = sensorsSetup,
    .run         = sensorsRun,
    .enabled     = true
};

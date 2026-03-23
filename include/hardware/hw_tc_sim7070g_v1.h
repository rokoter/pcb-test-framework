/**
 * @file hw_tc_sim7070g_v1.h
 * @brief Hardware-configuratie: Thermocouple board met SIM7070G v1.0
 *
 * MCU: ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
 * Modem: SIM7070G (Cat-M / NB-IoT)
 *
 * Verschil met SIM7080G variant:
 *   - SIM7070G kan NIET tegelijk cellular + GNSS (belangrijk voor test flow)
 *   - PIN_SIM_CD (SIM card detect) op IO42 i.p.v. SD_DATA3
 *   - Geen RTS/CTS flow control
 */

#pragma once

// ── Board identificatie ─────────────────────────────────────────────────────
#define HW_BOARD_NAME       "Thermocouple SIM7070G v1.0"
#define HW_MCU              "ESP32-S3-WROOM-1-N16R8"

// ── LTE Modem — SIM7070G ────────────────────────────────────────────────────
#define PIN_SIM_TXD         17
#define PIN_SIM_RXD         18
#define PIN_SIM_PWR         45
#define PIN_SIM_STATUS      41
#define PIN_SIM_CD          42   // SIM card detect (niet op SIM7080G board)
#define MODEM_BAUD          115200
#define MODEM_HAS_RTS_CTS   false
// Geen RTS/CTS/DTR/RI op dit board

// ── SPI — Thermocouple ADC (3x MAX31856MUD+T) ──────────────────────────────
#define PIN_SPI_MOSI        11
#define PIN_SPI_MISO        13
#define PIN_SPI_SCK         12
#define PIN_TC1_CS          10
#define PIN_TC2_CS           9
#define PIN_TC3_CS          46
#define PIN_SENSORS_PWR     35   // TPS22917DBVR — 3.3V schakelaar voor TC

// ── I2C Bus 1 — Fuel Gauge (BQ27220YZFR) ───────────────────────────────────
#define PIN_I2C1_SDA         8
#define PIN_I2C1_SCL         2
#define I2C1_FUEL_GAUGE_ADDR 0x55

// ── I2C Bus 2 — Charge Controller (MP2731GQC) + Moisture sensor ────────────
#define PIN_I2C2_SDA        15
#define PIN_I2C2_SCL        16
#define I2C2_CHARGE_ADDR    0x4B

// ── SD-kaart (4-bit SDIO) ───────────────────────────────────────────────────
#define PIN_SD_CLK          39
#define PIN_SD_CMD          40
#define PIN_SD_DATA0        38
#define PIN_SD_DATA1        37
#define PIN_SD_DATA2        43   // TXD0
#define PIN_SD_DATA3        42

// ── 1-Wire — DS18B20 temperatuursensoren ────────────────────────────────────
#define PIN_ONEWIRE_1       14
#define PIN_ONEWIRE_2       21

// ── Overige signalen ────────────────────────────────────────────────────────
#define PIN_NTC1             1   // Analoge NTC (ADC)
#define PIN_GPS_POWER       36   // Actieve GPS antenne aan/uit
#define PIN_KNOP_LED        47   // LED op knop
#define PIN_USER_LED        48   // Status LED

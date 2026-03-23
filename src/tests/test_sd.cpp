/**
 * @file test_sd.cpp
 * @brief Testmodule: SD-kaart (1-bit en 4-bit SDIO)
 *
 * Test mount, functioneel lezen/schrijven, en benchmark.
 * Pinnen komen automatisch uit hardware.h — werkt op elke revisie.
 */

#include "hardware.h"
#include "test_module.h"

#if HAS_SD_CARD

#include "SD_MMC.h"
#include "FS.h"

// ── Benchmark instellingen ──────────────────────────────────────────────────
#define BENCH_FILE     "/bench.bin"
#define BENCH_SIZE_KB  512
#define BENCH_CHUNK_KB 16

static uint8_t chunkBuf[BENCH_CHUNK_KB * 1024];

// ── Interne functies ────────────────────────────────────────────────────────

static bool mountCard(bool mode1bit, int freqKHz) {
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_DATA0,
                   PIN_SD_DATA1, PIN_SD_DATA2, PIN_SD_DATA3);
    return SD_MMC.begin("/sdcard", mode1bit, false, freqKHz * 1000);
}

static void printCardInfo() {
    sdcard_type_t type = SD_MMC.cardType();
    const char* typeStr = "ONBEKEND";
    if      (type == CARD_MMC)  typeStr = "MMC";
    else if (type == CARD_SD)   typeStr = "SDSC";
    else if (type == CARD_SDHC) typeStr = "SDHC/SDXC";
    Serial.printf("  Kaarttype    : %s\n", typeStr);
    Serial.printf("  Kaartgrootte : %llu MB\n", SD_MMC.cardSize() / (1024ULL * 1024ULL));
}

static bool runFunctionalTest() {
    const char* path    = "/functest.txt";
    const char* content = "ESP32-S3 SDIO functionele test OK";

    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) { Serial.println("  FAIL: schrijven mislukt"); return false; }
    f.print(content); f.close();

    f = SD_MMC.open(path, FILE_READ);
    if (!f) { Serial.println("  FAIL: lezen mislukt"); return false; }
    String read = "";
    while (f.available()) read += (char)f.read();
    f.close(); SD_MMC.remove(path);
    read.trim();

    if (read != String(content)) { Serial.println("  FAIL: inhoud klopt niet"); return false; }
    Serial.println("  OK: schrijven en teruglezen correct");
    return true;
}

static void runBenchmark() {
    const size_t chunkSize  = BENCH_CHUNK_KB * 1024;
    const size_t totalBytes = BENCH_SIZE_KB  * 1024;
    const int    chunks     = BENCH_SIZE_KB  / BENCH_CHUNK_KB;

    for (size_t i = 0; i < chunkSize; i++) chunkBuf[i] = (uint8_t)(i & 0xFF);

    File f = SD_MMC.open(BENCH_FILE, FILE_WRITE);
    if (!f) { Serial.println("  Benchmark: kan bestand niet openen"); return; }

    uint32_t t0 = millis();
    for (int i = 0; i < chunks; i++) f.write(chunkBuf, chunkSize);
    f.flush(); f.close();
    uint32_t writeMs = millis() - t0;

    f = SD_MMC.open(BENCH_FILE, FILE_READ);
    if (!f) { SD_MMC.remove(BENCH_FILE); return; }
    t0 = millis();
    size_t totalRead = 0;
    while (f.available()) totalRead += f.read(chunkBuf, chunkSize);
    uint32_t readMs = millis() - t0;
    f.close(); SD_MMC.remove(BENCH_FILE);

    float writeKBs = (float)BENCH_SIZE_KB / ((float)writeMs / 1000.0f);
    float readKBs  = (float)BENCH_SIZE_KB / ((float)readMs  / 1000.0f);
    Serial.printf("  Schrijven : %7.1f KB/s\n", writeKBs);
    Serial.printf("  Lezen     : %7.1f KB/s\n", readKBs);
}

// ── Module interface ────────────────────────────────────────────────────────

static bool sdSetup() {
    // Niets speciaals nodig — SD_MMC.begin() gebeurt in run()
    return true;
}

static bool sdRun() {
    printHeader("SD-KAART TEST");
    bool anyPassed = false;

    // Test 1-bit modus (meest compatibel)
    Serial.println("\n  [1-bit @ 400 kHz]");
    if (mountCard(true, 400)) {
        printCardInfo();
        anyPassed = runFunctionalTest();

        Serial.printf("  Benchmark (%d KB):\n", BENCH_SIZE_KB);
        runBenchmark();
        SD_MMC.end();
    } else {
        Serial.println("  FAIL: mount mislukt — kaart aanwezig en FAT32?");
    }

    delay(100);

    // Test 4-bit modus
    Serial.println("\n  [4-bit @ 20 MHz]");
    if (mountCard(false, 20000)) {
        Serial.println("  OK: 4-bit SDIO mount geslaagd");
        runFunctionalTest();

        Serial.printf("  Benchmark (%d KB):\n", BENCH_SIZE_KB);
        runBenchmark();
        SD_MMC.end();
    } else {
        Serial.println("  4-bit mount mislukt (check pull-ups op DATA1-3)");
    }

    return anyPassed;
}

// ── Registratie ─────────────────────────────────────────────────────────────
TestModule testSD = {
    .name        = "SD-kaart",
    .description = "1-bit/4-bit SDIO mount, lees/schrijf, benchmark",
    .setup       = sdSetup,
    .run         = sdRun,
    .enabled     = true
};

#endif // HAS_SD_CARD

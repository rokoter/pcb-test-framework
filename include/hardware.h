/**
 * @file hardware.h
 * @brief Centrale hardware-include — kiest automatisch de juiste configuratie.
 *
 * Dit bestand wordt overal ge-include in plaats van een specifiek hw_*.h bestand.
 * De platformio.ini build_flags bepalen welk bestand geladen wordt.
 *
 * Gebruik in je code:
 *   #include "hardware.h"
 *   // PIN_SIM_TXD, PIN_SD_CLK, etc. zijn nu beschikbaar
 */

#pragma once

// ── Laad hardware-configuratie op basis van build flags ─────────────────────
#if defined(HW_CONFIG_FILE)
    // Stringify macro trick: HW_CONFIG_FILE bevat het pad als string
    #include HW_CONFIG_FILE
#else
    #error "Geen HW_CONFIG_FILE gedefinieerd! Kies een environment in platformio.ini."
#endif

// ── Validatie: controleer dat essentiële pins gedefinieerd zijn ─────────────
#ifndef HW_BOARD_NAME
    #error "HW_BOARD_NAME niet gedefinieerd in hardware config!"
#endif

// ── Hulpmacro's voor conditionele compilatie ────────────────────────────────
// Gebruik deze in testmodules om features te checken:
//   #if HAS_MODEM
//       runModemTest();
//   #endif

#if defined(MODEM_TYPE_SIM7080G) || defined(MODEM_TYPE_SIM7070G)
    #define HAS_MODEM 1
#else
    #define HAS_MODEM 0
#endif

#ifdef BOARD_HAS_SD_CARD
    #define HAS_SD_CARD 1
#else
    #define HAS_SD_CARD 0
#endif

#ifdef BOARD_HAS_FUEL_GAUGE
    #define HAS_FUEL_GAUGE 1
#else
    #define HAS_FUEL_GAUGE 0
#endif

#ifdef BOARD_HAS_CHARGE_CONTROLLER
    #define HAS_CHARGE_CONTROLLER 1
#else
    #define HAS_CHARGE_CONTROLLER 0
#endif

#ifdef BOARD_HAS_THERMOCOUPLE
    #define HAS_THERMOCOUPLE 1
    #define TC_COUNT BOARD_HAS_THERMOCOUPLE
#else
    #define HAS_THERMOCOUPLE 0
    #define TC_COUNT 0
#endif

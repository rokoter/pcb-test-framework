# PCB Test Framework

## Wat is dit project?
Modulair testframework voor ESP32-S3-gebaseerde PCB's met diverse hardware-revisies.
Eén firmware, meerdere testmodules, configureerbaar per board via `platformio.ini`.

## Architectuur
- `platformio.ini` — Eén environment per hardware-revisie (board + modem + features)
- `include/hardware.h` — Centrale include, routeert naar het juiste `hw_*.h` bestand
- `include/hardware/hw_*.h` — Pinout en features per revisie
- `include/test_module.h` — Interface die alle testmodules implementeren
- `src/main.cpp` — Serial menu + test runner
- `src/tests/test_*.cpp` — Individuele testmodules (SD, modem, sensoren, etc.)

## Conventies
- **Taal code**: C++ (Arduino framework), comments in het Nederlands
- **Pins**: altijd via `PIN_*` defines uit hardware.h, NOOIT hardcoded nummers
- **Features**: gebruik `HAS_*` macro's voor conditionele compilatie
- **Nieuwe test toevoegen**: maak `src/tests/test_naam.cpp`, implementeer TestModule struct, voeg extern reference toe in `main.cpp`
- **Nieuwe hardware-revisie**: kopieer een `hw_*.h`, pas pinnen aan, voeg env toe in `platformio.ini`

## Commando's
- `pio run -e thermocouple_sim7080g_v1` — Compileer voor specifiek board
- `pio run -e thermocouple_sim7080g_v1 -t upload` — Upload naar ESP32
- `pio device monitor` — Serial monitor openen

## Hardware
- MCU: ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
- Modems: SIM7080G (Cat-M/NB-IoT) of SIM7070G
- Sensoren: MAX31856 thermocouples, BQ27220 fuel gauge, MP2731 charge controller
- Opslag: SD-kaart via 4-bit SDIO
- Extra: DS18B20 1-Wire, NTC, GPS antenne, LEDs

## Projectregels
Zie `docs/PROJECT_RULES.md` — LEES DIT EERST bij elke nieuwe sessie.
Bevat: security regels (geen secrets in git), communicatiestijl (Nederlands),
code-kwaliteitsregels, en een checklist voor AI-assistenten.
Dit bestand is AI-agnostisch en geldt ook als je wisselt van model/tool.

## Secrets
Gevoelige data (WiFi, APN, MQTT, etc.) hoort in `include/secrets.h`.
Dit bestand staat in `.gitignore`. Gebruik `include/secrets.h.example` als template.

## Roadmap
Zie `docs/ROADMAP.md` voor toekomstige plannen (QR-code identificatie,
test-naar-module pipeline, KiCad integratie, hardware design assistent).
Lees dit bestand als de gebruiker vraagt om nieuwe features.

## Git workflow
- `main` branch = stabiel en werkend
- Feature branches: `feature/naam-van-feature`
- Commit messages: imperatief, Nederlands, kort en beschrijvend

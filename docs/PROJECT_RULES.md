# Projectregels — PCB Test Framework

Dit bestand bevat regels die ALTIJD gelden, ongeacht welke AI, welk model,
of welke versie wordt gebruikt. Elke AI-assistent (Claude Code, Copilot,
ChatGPT, toekomstige tools) MOET dit bestand lezen en respecteren.

Laatste update: maart 2026

---

## 1. Beveiliging & gevoelige data

### Nooit in Git / GitHub
- Geen API keys, tokens, wachtwoorden, of credentials
- Geen WiFi SSID's of wachtwoorden
- Geen persoonlijke gegevens (namen, adressen, telefoonnummers)
- Geen APN-instellingen met accountgegevens
- Geen serienummers van productie-apparaten
- Geen interne bedrijfsinformatie of klantgegevens

### Hoe dan wel?
- Gebruik `secrets.h` (staat in `.gitignore`, wordt NOOIT gecommit)
- Template: `secrets.h.example` met placeholder-waarden WEL in Git
- Environment variables voor CI/CD
- `.env` bestanden (in `.gitignore`)

### Voorbeeld secrets.h.example
```cpp
// Kopieer dit bestand naar secrets.h en vul je eigen waarden in.
// secrets.h staat in .gitignore en wordt NIET gecommit.
#pragma once

#define WIFI_SSID       "VERVANG_MIJ"
#define WIFI_PASSWORD   "VERVANG_MIJ"
#define APN_NAME        "VERVANG_MIJ"
#define MQTT_SERVER     "VERVANG_MIJ"
#define MQTT_USER       "VERVANG_MIJ"
#define MQTT_PASSWORD   "VERVANG_MIJ"
```

### Security best practices
- OTA updates: altijd gesigned, met rollback-mogelijkheid
- Seriële debug-output: geen credentials printen, ook niet bij verbose logging
- SD-kaart logs: geen secrets opslaan in plaintext
- Modem commando's: AT+CPIN en APN-configuratie loggen ZONDER wachtwoorden
- Bij twijfel: vraag de gebruiker, commit NIET automatisch

---

## 2. Communicatiestijl

### Code & comments
- Taal: Nederlands voor comments, documentatie, en serial output
- Variabelenamen: Engels (standaard in C/C++ ecosysteem)
- Commit messages: Nederlands, imperatief, kort
  - Goed: "Voeg GPS testmodule toe"
  - Fout: "Added GPS test module" of "ik heb de GPS test toegevoegd"
- Geen emoji in code of comments
- Serial output: Nederlands, duidelijk voor iemand die niet programmert

### AI-interactie
- Spreek de gebruiker aan als "je/jij" (informeel)
- Leg technische keuzes uit in begrijpelijke taal
- Bij onzekerheid: vraag, neem niet aan
- Stel alternatieven voor in plaats van alleen "dit kan niet"
- Geef context bij waarschuwingen ("dit is gevaarlijk OMDAT...")

### Documentatie
- README's en handleidingen: Nederlands
- Code-documentatie (Doxygen-stijl): Nederlands
- Bestandsnamen: Engels (hw_tc_sim7080g_v1.h, niet hw_tc_sim7080g_v1.h)
- Mapnamen: Engels (include/, docs/, src/tests/)

---

## 3. Kwaliteitsregels voor code

### Altijd
- Pinnen via `PIN_*` defines uit hardware.h — NOOIT hardcoded nummers
- Features via `HAS_*` macro's — conditionele compilatie
- Elke testmodule implementeert de TestModule interface
- Foutafhandeling: geen stille failures, altijd serial output bij fouten
- Nieuwe code moet compileren voordat het gecommit wordt

### Nooit
- Geen `delay()` langer dan 5 seconden zonder uitleg waarom
- Geen `while(true)` zonder timeout of escape-conditie
- Geen globale variabelen tenzij strikt noodzakelijk
- Geen magic numbers — gebruik #define of const met duidelijke naam
- Geen code die alleen werkt op één specifieke hardware-revisie
  (gebruik altijd de hardware.h abstractie)

### Git workflow
- `main` branch = compileert, is getest, werkt
- Feature branches voor nieuwe functionaliteit: `feature/naam`
- Bugfix branches: `fix/naam`
- Nooit direct op main pushen bij grote wijzigingen
- Commit messages beschrijven WAT en WAAROM, niet HOE

---

## 4. AI-wisselbestendigheid

### Het probleem
Als je wisselt van AI (Claude → ChatGPT → Copilot → toekomstig model),
gaat context verloren. Dit bestand + CLAUDE.md + ROADMAP.md vormen samen
het "geheugen" van het project.

### De oplossing: alles staat in bestanden
- `CLAUDE.md` — projectstructuur, commando's, architectuur
  (naam is conventie, werkt met elke AI die het leest)
- `docs/ROADMAP.md` — toekomstplannen en context
- `docs/PROJECT_RULES.md` — dit bestand: harde regels
- `docs/lessons_learned.md` — opgedane kennis (wordt aangevuld)
- `secrets.h.example` — template voor gevoelige configuratie

### Bij het starten van een sessie met een nieuwe AI
Zeg: "Lees CLAUDE.md, docs/PROJECT_RULES.md, en docs/ROADMAP.md
voordat je begint."

### Wat elke AI moet weten
1. Dit is een embedded C++ project (ESP32-S3, PlatformIO, Arduino framework)
2. Er zijn meerdere hardware-revisies — gebruik altijd hardware.h
3. Nederlands is de voertaal voor alles behalve variabelenamen
4. Gevoelige data hoort NIET in Git — gebruik secrets.h
5. De roadmap bevat de toekomstplannen — lees die voor context

---

## 5. Checklist voor AI-assistenten

Voordat je code commit of een bestand aanmaakt, check:

- [ ] Bevat het geen API keys, wachtwoorden, of persoonlijke data?
- [ ] Zijn alle pinnen via hardware.h gedefinieerd (geen hardcoded nummers)?
- [ ] Is het conditioneel gecompileerd waar nodig (HAS_* macro's)?
- [ ] Compileert het zonder fouten?
- [ ] Zijn comments en serial output in het Nederlands?
- [ ] Is de commit message in het Nederlands en beschrijvend?
- [ ] Staat secrets.h (zonder .example) in .gitignore?
- [ ] Zijn er geen TODO's achtergebleven die security-gevoelig zijn?

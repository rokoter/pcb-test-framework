# Roadmap — PCB Test Framework

Dit document beschrijft de toekomstige richting van het framework.
Het is bedoeld als input voor Claude Code: bij elke sessie kan je zeggen
"pak de roadmap erbij" en Claude weet wat de plannen zijn.

---

## Fase 1 — Fundament (nu gereed)
- [x] Modulair testframework met serial menu
- [x] Hardware-configuratie per revisie via `hw_*.h`
- [x] PlatformIO met environments per board
- [x] Testmodules: SD-kaart, modem, sensoren
- [x] CLAUDE.md voor projectcontext
- [x] Setup-handleiding voor nieuwe PC

---

## Fase 2 — QR-code hardware-identificatie

### Concept
Elk PCB krijgt een QR-code (sticker of silkscreen) die het board identificeert.
Bij het scannen wordt automatisch de juiste hardware-revisie geladen.

### Hoe het werkt
- **QR-code inhoud**: simpele string, bijv. `TC-SIM7080G-V1.0-SN0042`
  - Product-code + revisie + serienummer
  - Formaat: `{PRODUCT}-{MODEM}-{VERSIE}-{SERIENUMMER}`
- **Scanner**: smartphone app (of een barcode scanner via USB-serial)
- **Koppeling**: serial commando stuurt QR-string naar ESP32
  - ESP32 matcht de product/revisie-code tegen een lookup-table
  - Laadt automatisch de juiste testset voor dat board
- **Dev mode**: als de QR-code niet herkend wordt, biedt het systeem aan:
  - Interactief pinout invoeren via serial menu
  - Een nieuw `hw_*.h` bestand genereren (met hulp van Claude Code)
  - De QR-code registreren voor toekomstig gebruik

### Technisch
- Lookup-table als `const struct` array in firmware, of JSON op SD-kaart
- Serial protocol: `QR:TC-SIM7080G-V1.0-SN0042\n`
- Dev mode schrijft configuratie naar SD-kaart als JSON/INI
- Claude Code kan op basis van een nieuwe QR-scan automatisch een hw_*.h voorstellen

### Vragen om later uit te werken
- Wil je de QR-code scannen via een telefoon-app die via BLE/WiFi communiceert?
- Of een USB barcode scanner die als serial keyboard werkt?
- Moet het serienummer ook gelogd worden (traceability)?

---

## Fase 3 — Test-naar-module pipeline ("test het, leer het, lever het")

### Concept
Een succesvolle test resulteert niet alleen in een PASS/FAIL, maar ook in
een werkende softwaremodule die door een IoT-ontwikkelaar gebruikt kan worden.
De brug tussen hardware-test en productie-software.

### Flow
```
Hardware tester          Claude Code            IoT developer
     |                       |                       |
     | 1. Draait test        |                       |
     |    (bijv. MP2731      |                       |
     |     charge controller)|                       |
     |                       |                       |
     | 2. Serial output:     |                       |
     |    registers, waarden |                       |
     |    foutstatus, etc.   |                       |
     |                       |                       |
     | 3. Bespreekt met      |                       |
     |    Claude Code:       |                       |
     |    "Ik wil dat het    |                       |
     |     laadt op 500mA    |                       |
     |     max, 4.2V cutoff" |                       |
     |                       |                       |
     |              4. Claude genereert:              |
     |                 - settings.json/h met alle     |
     |                   registerwaardes              |
     |                 - mp2731_driver.h/cpp module   |
     |                   met init(), setChargeCurrent,|
     |                   getStatus(), etc.            |
     |                 - Documentatie van wat elk      |
     |                   register doet                |
     |                       |                       |
     |                       | 5. IoT dev gebruikt:  |
     |                       |    #include "mp2731.h" |
     |                       |    mp2731.init();      |
     |                       |    mp2731.setCharge    |
     |                       |      Current(500);     |
```

### Concreet: settings-bestand generatie
- Na een succesvolle I2C test, log alle register-waarden
- Claude Code leest de serial output + datasheet-kennis
- Jij geeft het doel: "laadstroom 500mA, max spanning 4.2V"
- Claude genereert:
  - `config/mp2731_settings.h` — alle register-waarden als #defines
  - `lib/mp2731/mp2731.h` + `mp2731.cpp` — kant-en-klare driver
  - `docs/mp2731_configuratie.md` — uitleg voor de IoT developer
- Settings-bestand bevat ook metadata: op welk board het getest is,
  wanneer, met welke firmware-versie

### Leerpunten-systeem
- Elke test kan een `lessons_learned.md` bijwerken
- Voorbeelden:
  - "BQ27220 geeft 0xFFFF als er geen batterij is — handle dit"
  - "SIM7070G moet cellular uit voor GPS — voeg dit toe aan modem driver"
  - "4-bit SDIO vereist pull-ups op DATA1-3, anders alleen 1-bit"
- Claude Code kan deze lessons lezen bij het genereren van nieuwe modules
- Bij toekomstige revisies: "check of dit board dezelfde valkuilen heeft"

---

## Fase 4 — Hardware design assistent

### Concept
Claude Code als sparringpartner bij het ontwerpen van nieuwe boards.
Niet als vervanging van KiCad, maar als adviseur die de context kent.

### Mogelijkheden

#### Component selectie
- "Ik zoek een charge controller voor 2S LiPo, I2C, max 2A"
- Claude doorzoekt zijn kennis + web, stelt opties voor met trade-offs
- Vergelijkt met wat je al gebruikt (MP2731) — "deze is pin-compatible"
- Kan zelfs een eerste hw_*.h voorstellen voor het nieuwe IC

#### Pinout review
- "Check of mijn pinout-toewijzing klopt voor de ESP32-S3"
- Claude kent de beperkingen: strapping pins, input-only pins, ADC kanalen
- Waarschuwt voor conflicten: "IO42 is JTAG, werkt maar als je JTAG uitschakelt"

#### KiCad integratie
- **Realistisch op korte termijn:**
  - Claude Code kan `.kicad_sch` bestanden lezen (het is XML-achtig)
  - Pinout extractie: "welke pinnen zijn verbonden in dit schema?"
  - BOM generatie: "lijst alle unieke componenten"
  - Vergelijking: "wat is veranderd tussen schema v1.0 en v1.1?"
  - Automatisch een hw_*.h genereren vanuit het KiCad schema
- **Realistisch op middellange termijn:**
  - KiCad CLI (`kicad-cli`) kan schema's exporteren naar netlist/BOM
  - Claude Code kan dit aanroepen en de output interpreteren
  - DRC (Design Rule Check) output lezen en samenvatten
- **Minder realistisch (nu):**
  - Directe wijzigingen in KiCad-bestanden (te complex/fragiel)
  - PCB layout suggesties (dat is een visueel-ruimtelijk probleem)

#### Workflow
```
Jij: "Ik wil een nieuwe revisie met een SIM7600 in plaats van SIM7080G"

Claude Code:
1. Zoekt SIM7600 specificaties op
2. Vergelijkt pinout met SIM7080G
3. Stelt een nieuwe hw_sim7600_v1.h voor
4. Waarschuwt: "SIM7600 heeft hogere stroompieken, check je voeding"
5. Maakt een test_modem_4g.cpp met aangepaste AT-commando's
6. Als je een KiCad schema deelt: trekt de werkelijke pinout eruit
   en controleert die tegen de voorgestelde hw_*.h
```

---

## Fase 5 — Remote testing (testen op afstand)

### Concept
Na eerste USB-C upload het board de wereld in sturen en op afstand
testen uitvoeren, resultaten ophalen, en firmware updaten.
Use case: LoRa-ontvangst testen op verschillende locaties zonder laptop.

### Mogelijke aanpakken

#### WiFi debug netwerk
- ESP32-S3 start een eigen WiFi AP ("PCB-TEST-xxxx")
- Simpele webinterface: telefoon verbindt, ziet het testmenu
- Serial output wordt gestreamd naar de browser (WebSocket)
- Tests starten/stoppen via de telefoon
- Resultaten opslaan op SD-kaart + tonen in browser

#### Via het modem (LTE/NB-IoT)
- Board stuurt testresultaten naar een server via MQTT/HTTP
- Commando's ontvangen: "draai LoRa bereiktest"
- Periodiek automatische tests op een timer
- Locatie via GPS meesturen → kaart van testpunten

#### OTA (Over-The-Air) firmware updates
- Nieuwe testfirmware pushen zonder fysieke toegang
- Vereist: OTA partitie-schema in platformio.ini
- Veiligheid: signed updates, rollback bij mislukte flash

### Use case: LoRa bereiktest
```
1. Upload firmware via USB-C met WiFi AP + LoRa test module
2. Plaats board op locatie A
3. Loop met telefoon naar locatie B, C, D...
4. Op elke locatie: verbind met board's WiFi AP via telefoon
5. Start LoRa bereiktest → board stuurt pakketjes, meet RSSI/SNR
6. Resultaten + GPS-coördinaten worden gelogd op SD-kaart
7. Thuis: SD-kaart uitlezen → kaart met dekkingsgebied
```

### Alternatief: volledig autonoom
```
1. Board draait op batterij met GPS + modem
2. Elke X minuten: LoRa test + GPS fix + upload resultaat via LTE
3. Dashboard toont live kaart met meetpunten en signaalsterkte
4. Geen telefoon/laptop nodig in het veld
```

---

## Fase 6 — Productie-test modus

### Later, als het framework volwassen is
- One-click test: QR scannen → alle tests draaien → rapport genereren
- Test-rapport opslaan op SD-kaart als JSON/CSV
- Pass/fail criteria per test configureerbaar
- Statistieken bijhouden: "van de 50 boards van rev 2, faalden 3 op SD-kaart"
- Mogelijk: resultaten uploaden via modem naar een server/dashboard

---

## Fase 7 — Remote ontwikkelen met Claude Code

### Het probleem
Je wilt Claude Code taken laten uitvoeren op een remote machine (thuis-PC,
VPS, of dev-server) terwijl jij onderweg bent met een slechte verbinding,
of zelfs als je lokale sessie wegvalt. Claude moet gewoon doorwerken.

### Wat nu al werkt (maart 2026)

#### Optie A: SSH + tmux (simpelst, werkt nu)
```
Jouw laptop/telefoon ──SSH──> Thuis-PC met tmux
                                  └── Claude Code draait hier
```
- Claude Code draait in een `tmux` sessie op je thuis-PC / server
- Je SSH't erin vanuit je laptop, telefoon (Termux/Blink), of tablet
- Als je verbinding wegvalt: tmux houdt de sessie in leven
- Je reconnect later en ziet precies waar Claude gebleven is
- **Data-gebruik**: minimaal, alleen terminal-tekst over SSH
- **Setup**: `sudo dnf install tmux` + SSH-server configureren

Werkflow:
```bash
# Op je server/thuis-PC (eenmalig):
tmux new-session -s claude

# In de tmux sessie:
cd ~/projects/pcb-test-framework
claude

# Later, vanaf je telefoon/laptop:
ssh jouw-server
tmux attach -t claude
# → Je ziet Claude precies waar je hem achterliet
```

#### Optie B: SSH + tmux + Tailscale (geen port forwarding nodig)
```
Jouw telefoon ──Tailscale VPN──> Thuis-PC
                                      └── tmux → Claude Code
```
- Tailscale maakt een privé-netwerk tussen je apparaten
- Geen poorten openen in je router, geen dynamisch DNS
- Werkt ook als je thuis-PC achter NAT zit
- Gratis voor persoonlijk gebruik (tot 100 apparaten)

#### Optie C: Claude Code Remote Control (nieuwste feature)
```
Jouw telefoon ──claude.ai/code──> Claude Code op server
```
- Start Claude Code met `claude` op je server (via tmux)
- Open claude.ai/code op je telefoon of browser
- Je ziet dezelfde sessie, inclusief goedkeurings-prompts
- Bevestigd werkend via SSH + tmux op headless Linux
- **Voordeel**: mooiere interface dan raw terminal op telefoon

#### Optie D: Headless mode (voor automatisering)
```bash
# Laat Claude een taak uitvoeren zonder interactie:
ssh server "cd ~/projects/pcb-test-framework && claude -p 'Draai alle tests en fix compileerfouten'"

# Output als JSON voor scripts:
ssh server "claude -p 'Lijst alle hardware revisies' --output-format json"
```
- `-p` flag: stuur een prompt, krijg antwoord, klaar
- Perfect voor geautomatiseerde taken (CI/CD, cronjobs)
- Geen interactie nodig — je verbinding mag wegvallen VOOR de start

### Aanbeveling voor jouw situatie
Start met **Optie A** (SSH + tmux) — nul extra tools, werkt direct.
Voeg later Tailscale toe als je ook buiten je thuisnetwerk wilt werken.

### Beperkingen (eerlijk zijn)
- Bij een interactieve sessie moet je af en toe goedkeuring geven
  (Claude vraagt "mag ik dit bestand wijzigen?"). Als je verbinding
  precies dan wegvalt, wacht Claude tot je terugkomt.
- Headless mode (`-p`) heeft dit probleem niet, maar is minder flexibel
- Er is een feature request voor `--headless` remote control
  (Claude draait als daemon, je telefoon is de enige UI) — nog niet
  officieel beschikbaar maar wordt actief besproken

### Wat je nodig hebt
- Een machine die altijd aan staat (thuis-PC, Raspberry Pi, VPS)
- SSH-server (`sudo dnf install openssh-server`)
- tmux (`sudo dnf install tmux`)
- Optioneel: Tailscale (tailscale.com, gratis tier)
- Claude Code geïnstalleerd op de remote machine

---

## Prioritering

| Fase | Onderwerp              | Impact | Moeite | Wanneer         |
|------|------------------------|--------|--------|-----------------|
| 1    | Fundament              | Hoog   | Laag   | Nu (gedaan)     |
| 2    | QR-code identificatie  | Middel | Laag   | Volgende sprint |
| 3    | Test-naar-module       | Hoog   | Middel | Na fase 2       |
| 4    | Hardware design assist | Hoog   | Middel | Parallel aan 3  |
| 5    | Remote testing (board) | Hoog   | Middel | Na fase 3       |
| 6    | Productie-test modus   | Middel | Hoog   | Later           |
| 7    | Remote development     | Hoog   | Laag   | Wanneer je wilt |

---

## Hoe te gebruiken met Claude Code

Open Claude Code in je projectmap en zeg bijvoorbeeld:
- "Lees de roadmap en begin aan fase 2 — QR-code identificatie"
- "Ik wil de test-naar-module pipeline opzetten voor de BQ27220"
- "Help me de KiCad integratie uitwerken — hier is mijn schema"

Claude Code leest dit bestand, kent de context, en kan meteen aan de slag.

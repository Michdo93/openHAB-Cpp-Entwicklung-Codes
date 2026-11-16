# openHAB & C++-Entwicklung – Begleit-Repository

Beispiele zum Buch „openHAB & C++-Entwicklung“ (Reihe „openHAB REST API“). Alle Programme
verwenden [cpp-openhab-rest-client](https://github.com/Michdo93/cpp-openhab-rest-client) und,
für die Tests, [cpp-openhab-test-suite](https://github.com/Michdo93/cpp-openhab-test-suite) in
Version 2.0.0 – CMake holt beide beim ersten Übersetzen von GitHub.

## Voraussetzungen

- Linux (Ubuntu 24.04/26.04, Debian, Raspberry Pi OS) oder WSL; C++17-Compiler (GCC 9+,
  Clang 10+), CMake 3.16+, Git
  ```bash
  sudo apt install build-essential cmake git libcurl4-openssl-dev nlohmann-json3-dev \
                   qt6-base-dev libgtest-dev libgmock-dev
  ```
  Ohne Qt 6 entfällt nur Kapitel 17, ohne GoogleTest nur ein Teil von Kapitel 19.
- openHAB 5 mit der Übungskonfiguration aus `openhab-conf/` (Kapitel 1 des Buchs)
- ein API-Token (Main UI → Profil → API-Tokens)

## Installation

```bash
git clone https://github.com/Michdo93/openHAB-Cpp-Entwicklung-Codes.git
cd openHAB-Cpp-Entwicklung-Codes
cp .env.example .env                 # Adresse und Token eintragen
./starte.sh kapitel10_erste_schritte/01_verbinden.cpp
```

`starte.sh` übersetzt beim ersten Aufruf alle Beispiele nach `build/`, lädt `.env` und startet
das gewünschte Programm. Von Hand: `cmake -S . -B build && cmake --build build -j2`, danach
liegt jedes Beispiel unter `build/<ordner>/<name>`. Die Datei `.env` steht in `.gitignore` –
Tokens gehören nicht ins Repository.

## Inhalt

| Ordner | Inhalt |
|---|---|
| `kapitel08_cpp/` | Werte und Typen, Kontrollfluss, Funktionen, Klassen und RAII, Container und Lambdas, Fehler und `std::optional`, optionale Argumente |
| `kapitel09_werkzeuge/` | nlohmann/json, libcurl, Smart Pointer, Fehlersuche (Sanitizer, valgrind), Threads, Signale, Dateien und Prozesse |
| `kapitel10_erste_schritte/` | Verbinden, Items lesen, Commands, States, Fehler, parallele Anfragen, optionale Parameter |
| `kapitel11_items/` | Items anlegen, viele Items, Metadaten, semantisches Modell, Gruppen und Verlauf |
| `kapitel12_things/` | Add-ons, Thing-Typen, Discovery und Inbox, Things, Links, Actions |
| `kapitel13_regeln/` | Regeln lesen, anlegen, steuern, Zeitpläne und Skripte, Modultypen |
| `kapitel14_daten/` | Persistenz, Testdaten, Verlauf als Textdiagramm, Daten löschen, Transformationen, CSV |
| `kapitel15_oberflaechen/` | Sitemaps lesen und anlegen, Sitemap-Ereignisse, Sprache, System |
| `kapitel16_ereignisse/` | SSE roh, Ereignisketten, Ereignisarten, State-Tracker, robuster Listener, Observer |
| `kapitel17_dashboard/` | `ohdash`: Qt-6-Dashboard nach Model-View-Controller (Fenster und Text) |
| `kapitel18_geraet/` | `ohgeraet`: der Raspberry Pi als Gerät, GPIO mit libgpiod 2, systemd-Unit |
| `kapitel19_tests/` | GoogleTest und GMock, Attrappe für openHAB, Integrationstests mit der Test-Suite, Vorab-Test |
| `kapitel20_projekte/` | `ohctl` für die Kommandozeile, ROS-2-Paket `ohbruecke`, Cross-Kompilierung |
| `openhab-conf/` | Übungskonfiguration (Items, Things, Regeln, Sitemap, Persistenz) |
| `.github/workflows/` | GitHub Actions: Tests mit einem frischen openHAB bei jedem Push |

## Programme aus mehreren Dateien

```bash
build/kapitel17_dashboard/ohdash                       # Fenster; --text für Zeilen
build/kapitel18_geraet/ohgeraet --intervall=30         # bis Strg+C; --ausgang=gpiochip0:17
build/kapitel20_projekte/ohctl items 'Wohnzimmer_*'
ctest --test-dir build -L unit                         # Tests ohne openHAB
ctest --test-dir build -L integration                  # Tests mit openHAB (.env laden)
```

| Option (CMake) | Wirkung |
|---|---|
| `-DMIT_ASAN=ON` | AddressSanitizer und UndefinedBehaviorSanitizer (Kapitel 9) |
| `-DMIT_GPIOD=ON` | `ohgeraet` schaltet eine echte GPIO-Leitung (libgpiod 2 mit C++-Anbindung) |
| `-DCMAKE_TOOLCHAIN_FILE=kapitel20_projekte/aarch64-linux-gnu.cmake` | für den Raspberry Pi (64 Bit) übersetzen |

## ROS 2 (Kapitel 20)

Das Paket `kapitel20_projekte/ros2/ohbruecke` wird mit colcon gebaut (ROS 2 Jazzy):

```bash
source /opt/ros/jazzy/setup.bash
colcon build --base-paths kapitel20_projekte/ros2
source install/setup.bash
set -a; . ./.env; set +a
ros2 run ohbruecke haus_bruecke --ros-args \
     --params-file kapitel20_projekte/ros2/ohbruecke/config/bruecke.yaml
```

## Zum Buch

„openHAB & C++-Entwicklung – Daemons, Qt-Oberflächen und Robotik mit der openHAB REST API,
Server-Sent Events und dem cpp-openhab-rest-client – modernes C++17 für das Smart Home“,
Michael Christian Dörflinger, Reihe „openHAB REST API“. Jede Datei, die im Buch abgedruckt
ist, liegt hier genau so.

> **Achtung:** Die Beispiele schalten Items und legen Items, Things, Regeln und Sitemaps an.
> Nur gegen eine Übungsinstanz mit `openhab-conf/` ausführen, nie gegen das eigene Zuhause.

## Lizenz

MIT – siehe [LICENSE](LICENSE).

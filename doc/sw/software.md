# Software-Architektur

## Zweck und Status

Diese Architektur beschreibt den minimalen Zephyr-/Zenoh-Demonstrator. Sie ist
eine Arbeitsgrundlage und wird mit jeder Lernstufe präzisiert. Sie schreibt
weder ein allgemeines Framework noch ein finales Produktprotokoll vor.

Die bisherige Basis steuert die Onboard-WS2812, stellt eine WLAN-Verbindung
mit DHCP her und öffnet eine Zenoh-Session zum Router auf dem Windows-Host.
zenoh-pico ist als gepinntes Zephyr-Modul mit drei dokumentierten Patches
eingebunden. Als nächstes wird Dilbert, das interaktive Zenoh-Werkzeug,
eingeführt; Home Assistant folgt erst danach.

## Grenzen des Systems

```text
Linux / WSL Host ── Wi-Fi / IP ── ESP32-S3 mit Zephyr ── Onboard-WS2812
                         Zenoh                 GPIO21
```

Der ESP32 verantwortet die Hardwareansteuerung und lokale, zeitkritische
Logik. Der Host sendet Commands und liest State, Events und Diagnosen. Zwischen
Host und Controller besteht keine Hard-Realtime-Anforderung.

## Komponenten

| Komponente | Verantwortung | Einführung |
| --- | --- | --- |
| `Application` | Orchestriert den Anwendungszustand und verarbeitet Commands. | sofort |
| `RgbLed` | Steuert die Onboard-WS2812 über die in Zephyr beschriebene Hardware. | sofort |
| `Connectivity` | Initialisiert und überwacht Netzwerkverbindung. | nach LED-Test |
| `ZenohClient` | Verwaltet Zenoh-Session, Reconnect sowie Ein- und Ausgaben. | nach Netzwerk |
| `Diagnostics` | Stellt nachvollziehbare serielle Diagnoseinformationen bereit. | bei Bedarf |

Diese Namen beschreiben Komponenten, keine vorab festgelegte Ordner- oder
Klassenstruktur. Abstrahieren wir erst, sobald ein konkreter Bedarf entsteht.

```text
          Command                         State / Event / Diagnose
Host ───────────────► ZenohClient ─► Application ─► RgbLed
Host ◄─────────────── ZenohClient ◄── Application
```

`RgbLed` kennt weder Zenoh noch Netzwerk. `ZenohClient` greift nicht direkt
auf Hardware zu. Damit bleiben Hardware- und Kommunikationslogik getrennt.

## Kommunikationsmodell

| Art | Bedeutung | Voraussichtliche Zenoh-Abbildung |
| --- | --- | --- |
| Command | Gewünschte Aktion | Query/Reply oder Pub/Sub; wird untersucht |
| State | Aktuell gültiger Zustand | Publish und bei Bedarf Query |
| Event | Eintretendes, flüchtiges Ereignis | Publish/Subscribe |
| Diagnose | Fehler und technische Laufzeitinformation | zunächst seriell, später optional Zenoh |

Synchroner Command kann mit Query/Reply abgebildet werden. Für längere,
asynchrone Vorgänge ist die Arbeitshypothese: Command/ACK über Query/Reply und
Abschluss als State oder Event. Diese Zuordnungen sowie Key Expressions und
Payload-Formate sind noch nicht festgelegt.

Nach einem Controller-Neustart sollen später Boot, Netzwerkaufbau,
Zenoh-Verbindung und State-Synchronisierung beobachtbar sein. Identity,
Firmware-/API-Version und Boot-ID sind mögliche Lifecycle-Daten, aber noch
kein festgelegter Vertrag.

## Zephyr- und C++-Vorgaben

- Hardwarekonfiguration erfolgt über Devicetree, Overlays, `prj.conf` und
  Kconfig; Anwendungscode enthält keine fest verdrahtete Boardbelegung.
- Ablaufsteuerung ist nicht blockierend. ISR-Routinen bleiben kurz; aufwändige
  Arbeit wird an geeignete Zephyr-Mechanismen übergeben.
- Komponenten kommunizieren über kleine, explizite Schnittstellen. Es gibt
  keine globalen Hardware- oder Zenoh-Zugriffe.
- Fehler werden als klare Rückgabewerte behandelt und seriell diagnostiziert.
- Heap-Allokation wird vermieden.
- Die Anwendung verwendet C++17. Weil `RgbLed` `std::array` nutzt, wird die
  vollständige GNU-C++-Standardbibliothek eingebunden; der damit verbundene
  Speicherbedarf wird bei späteren Firmware-Größenprüfungen berücksichtigt.

## Diagnose und Debugging

Serielle Zephyr-Logs sind der Standard für die erste Inbetriebnahme und für
Laufzeitdiagnosen. Sie werden vor einem Hardware-Debugger eingerichtet.

### Quellnavigation in VS Code

Die C/C++-Erweiterung (`ms-vscode.cpptools`) verwendet die nach einem Build
erzeugte Datei `build/hello-world/compile_commands.json`. Sie enthält die
exakten Compiler-Optionen sowie die Zephyr-, Treiber- und generierten Include-
Pfade. Die versionierte Workspace-Konfiguration
`.vscode/c_cpp_properties.json` verweist darauf. Dadurch funktioniert
"Gehe zu Definition" (F12) auch für Zephyr-Header wie
`<zephyr/kernel.h>`.

Nach einem frischen Klon muss zuerst `./scripts/build.sh` laufen. Danach in
VS Code einmal `Developer: Reload Window` ausführen, falls die Navigation
nicht unmittelbar aktualisiert wurde. Die C/C++-Erweiterung muss installiert
und aktiviert sein; CMake Tools ist dafür nicht erforderlich.

Für Quellcode-Debugging ist VS Code mit der Erweiterung
[IDE for Zephyr](https://docs.zephyrproject.org/latest/develop/tools/ide_for_zephyr_vscode_ext.html)
vorgesehen. Der Debug-Pfad lautet:

```text
VS Code → GDB → Espressif-OpenOCD → JTAG → ESP32-S3
```

Der ESP32-S3 benötigt eine Espressif-spezifische OpenOCD-Version; das in der
Zephyr SDK enthaltene OpenOCD unterstützt ESP32 nicht zwingend. Mit
`CONFIG_DEBUG_THREAD_INFO=y` kann OpenOCD Zephyr-Threads anzeigen.

Ob USB-JTAG über den USB-C-Anschluss des ESP32-S3-Zero mit dem gewählten
Zephyr-Target nutzbar ist, ist noch nicht verifiziert. Falls ein externer
JTAG-Debugger erforderlich wird, prüfen wir dessen Verdrahtung und
Konfiguration separat. Irreversible eFuse-Änderungen zur JTAG-Umschaltung
werden nicht vorgenommen, ohne dies vorher gemeinsam zu entscheiden.

### USB-Gerät aus Windows an WSL anhängen

Das ESP32-S3-Zero wird physisch an Windows angeschlossen. Damit Flash- und
serielle Werkzeuge in WSL darauf zugreifen können, wird das USB-Gerät mit
`usbipd-win` an die WSL-2-VM durchgereicht. Während es an WSL angehängt ist,
kann Windows nicht gleichzeitig auf das Gerät zugreifen.

Vor dem Anfügen bleibt ein WSL-Terminal geöffnet. Anschließend erfolgt das
Verfahren in zwei Umgebungen:

1. In einer als Administrator gestarteten Windows-PowerShell das Board suchen
   und anhand seiner Bus-ID einmalig freigeben:

   ```powershell
   usbipd list
   usbipd bind --busid <bus-id>
   ```

2. In einer normalen Windows-PowerShell das freigegebene Board an WSL
   anhängen:

   ```powershell
   usbipd attach --wsl --busid <bus-id>
   ```

   Bei wiederholten Hardware-Resets kann stattdessen der Auto-Attach-Modus
   verwendet werden:

   ```powershell
   usbipd attach --wsl --busid <bus-id> --auto-attach
   ```

   Der PowerShell-Prozess bleibt dabei geöffnet und hängt das Board nach einem
   USB-Reset erneut an WSL an. Diese Zuordnung gilt nur für die laufende Sitzung;
   nach einem Windows-Neustart oder dem Beenden des Prozesses muss sie erneut
   gestartet werden.

3. In WSL den Zugriff prüfen:

   ```bash
   lsusb
   ```

Nach erfolgreicher Prüfung können `west flash` und der serielle Monitor das
Gerät aus WSL verwenden. Nach der Arbeit wird es entweder physisch getrennt
oder in Windows-PowerShell wieder abgehängt:

```powershell
usbipd detach --busid <bus-id>
```

Die konkrete Bus-ID wird nicht dokumentiert, da sie sich beim Umstecken ändern
kann. Für das Verfahren sind WSL 2 und `usbipd-win` erforderlich.

## Preconditions

Vor dem ersten Build müssen die folgenden Voraussetzungen erfüllt und geprüft
sein:

- WSL mit einer unterstützten Linux-Distribution und VS Code mit WSL-Anbindung
- Zephyr-Host-Tools: mindestens CMake, Ninja, Python 3, Git und Devicetree
  Compiler (`dtc`)
- ein isoliertes Python Virtual Environment mit `west` und den
  Zephyr-Python-Abhängigkeiten
- ein separater Zephyr-Workspace mit Zephyr-Quellen und den benötigten Modulen
- installierte Zephyr SDK mit Xtensa-Toolchain für den ESP32-S3
- ein bestimmtes Zephyr-Board-Target einschließlich der zum ESP32-S3-Zero
  passenden Flash-, PSRAM- und Devicetree-Konfiguration
- für Flashen und Debugging: Zugriff von WSL auf die USB-Geräte

Die Anwendung verbleibt in diesem Repository; der Zephyr-Workspace wird
getrennt davon geführt. Die aktuelle Einrichtung folgt der
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/).

## Zephyr-Workspace installieren und verwenden

### Warum ein separater Workspace?

`nachtlicht` enthält ausschließlich unseren Anwendungscode und dessen
Dokumentation. Zephyr selbst besteht dagegen aus dem RTOS, Board-Definitionen,
Treibern, Build-Skripten und weiteren, versionsgebundenen Modulen. Diese
Bestandteile werden nicht in dieses Repository kopiert, weil sie umfangreich
sind und durch Zephyr gepflegt werden.

Der separate Workspace hält beide Verantwortlichkeiten klar getrennt:

```text
~/git/nachtlicht       unsere Anwendung, Konfigurationen und Dokumentation
~/git/zephyrproject    Zephyr-Quellen und die von Zephyr benötigten Module
```

Ein Build verwendet beide Bereiche: CMake liest die Anwendung aus
`nachtlicht`, findet Zephyr über den exportierten Workspace und erzeugt die
Firmware in einem Build-Verzeichnis. Weder der Workspace noch seine Module
werden von der Anwendung verändert.

### Was wird geklont?

`west init` klont zunächst das Zephyr-Repository. Darin liegt auch das
Manifest: eine Liste exakt definierter zusätzlicher Git-Repositories. `west
update` liest diese Liste und lädt die benötigten Module nach, beispielsweise
Hardware-Abstraktionsschichten für Espressif. Dadurch verwendet eine lokale
Zephyr-Version passende Versionen ihrer Abhängigkeiten statt beliebiger,
unabhängig heruntergeladener Bibliotheken.

Für den ESP32-S3 ist insbesondere der Espressif-HAL relevant. Zusätzlich
werden später Espressif-RF-Blobs benötigt, damit Wi-Fi funktionieren kann.
Sie werden separat mit `west blobs fetch hal_espressif` geladen und gehören
nicht zu unserem Anwendungscode.

### Installation

`west` ist in einem Python Virtual Environment installiert. Vor Zephyr-Arbeit
wird diese Umgebung im jeweiligen Terminal aktiviert:

```bash
source ~/.venvs/zephyr/bin/activate
```

Der Workspace wird einmalig unter `~/git/zephyrproject` angelegt und danach
vollständig eingerichtet:

```bash
west init -m https://github.com/zephyrproject-rtos/zephyr ~/git/zephyrproject
cd ~/git/zephyrproject
west update
west packages pip --install
west zephyr-export
west blobs fetch hal_espressif
```

`west packages pip --install` installiert die Python-Werkzeuge in Versionen,
die zum tatsächlich geklonten Zephyr-Stand passen. `west zephyr-export`
registriert diesen Stand für CMake, sodass unser unabhängiges
Anwendungs-Repository `find_package(Zephyr)` verwenden kann.

Die Zephyr SDK enthält den Cross-Compiler für die Xtensa-CPU des ESP32-S3.
Erst nach dem vollständigen Workspace-Download wird sie geprüft und
installiert. Damit stehen Compiler, Linker sowie zusätzliche Werkzeuge für
Build, Flash und Debugging konsistent zur Zephyr-Version bereit.

### Nutzung im Projekt

Für den ersten Konsolen-Test ist vorläufig
`esp32s3_devkitc/esp32s3/procpu` vorgesehen. Das Waveshare ESP32-S3-Zero hat
kein eigenes offizielles Zephyr-Target. Abweichende Eigenschaften wie Flash,
PSRAM und die WS2812 an GPIO21 werden daher vor der LED-Stufe mit einer lokalen
Board-Beschreibung oder einem Overlay erfasst.

Das Hello World weist ausschließlich nach, dass der Zephyr-Build, das Flashen
und die serielle Diagnose auf dem ESP32-S3 funktionieren. Erst danach wird die
Onboard-WS2812 angesteuert; Netzwerk und Zenoh bleiben bewusst außerhalb dieses
ersten Schritts.

## Hello World

### Ziel und Schnittstelle

Die erste Anwendung beweist den vollständigen minimalen Pfad von der
C++-Quelldatei bis zur seriellen Diagnose. `src/main.cpp` ist ihr einziger
Anwendungseinstieg. Die Funktion `main()` gibt nach dem Zephyr-Start genau eine
Diagnosezeile mit `printk()` aus und beendet sich anschließend. Zephyr hält das
System danach im Idle-Zustand am Leben.

Die Ausgabe ist keine fachliche Schnittstelle und besitzt keinen stabilen
Textvertrag. Sie dient ausschließlich als eindeutiger, von der Hardware
unabhängiger Inbetriebnahmenachweis.

### Konfiguration und Abgrenzung

`CMakeLists.txt` bindet die Anwendung über `find_package(Zephyr)` an den
separaten Zephyr-Workspace. `prj.conf` aktiviert C++ sowie die Konsolen- und
`printk()`-Ausgabe. Das vorläufige Build-Target lautet
`esp32s3_devkitc/esp32s3/procpu`. Da dessen Standardkonfiguration von 8 MB
Flash ausgeht, ergänzt der Build das Zephyr-Snippet `espressif-flash-4M` für
den tatsächlich vorhandenen 4-MB-Flash des ESP32-S3-Zero.

Hello World enthält absichtlich weder eine eigene Komponente noch
Threads, Netzwerk, Zenoh oder Hardwarezugriffe. Das minimale lokale
Board-Overlay aktiviert ausschließlich die eingebaute USB-Serial/JTAG-Schnittstelle
als Zephyr-Konsole. Dadurch wird die Ausgabe über den nativen USB-C-Anschluss
des Zero sichtbar, ohne GPIOs für den späteren WS2812-Treiber festzulegen.

### Verifikation

Der reproduzierbare pristine Build wird aus dem Zephyr-Workspace gestartet;
seine Artefakte bleiben im Anwendungs-Repository unter `build/hello-world`:

```bash
source ~/.venvs/zephyr/bin/activate
cd ~/git/zephyrproject/zephyr
west build -p always -d ~/git/nachtlicht/build/hello-world \
  -b esp32s3_devkitc/esp32s3/procpu -S espressif-flash-4M \
  ~/git/nachtlicht
```

Die erzeugte `zephyr.elf` dient dem Debugging; `zephyr.bin` ist die Firmware
für das Flashen. Beide sind abgeleitete Build-Artefakte und werden nicht in
Git versioniert. Der anschließende Hardwaretest umfasst Flashen und die
Beobachtung der Ausgabe über den seriellen Monitor. Der USB-Zugriff aus WSL
ist dafür noch separat zu prüfen.

Der Hardwaretest wurde am 11. September 2026 erfolgreich durchgeführt. Das
ESP32-S3-Zero wurde über USB-Serial/JTAG aus WSL erkannt, die Firmware mit
4-MB-Flash-Konfiguration geflasht und die Zephyr-Bootmeldung sowie
`Hello World from nachtlicht!` auf `/dev/ttyACM0` empfangen.

### Lokale Build-Hilfsskripte

Die wiederkehrenden Zephyr-Kommandos werden über Skripte im
Anwendungs-Repository aufgerufen. Damit bleibt `nachtlicht` der
Arbeitsordner; die Skripte wechseln selbst in den externen Zephyr-Workspace,
weil dort die `west`-Erweiterungen verfügbar sind.

| Skript | Aufgabe |
| --- | --- |
| `./scripts/build.sh` | Erstellt oder aktualisiert die 4-MB-USB-Serial/JTAG-Firmware. |
| `./scripts/flash.sh` | Flasht die vorhandene Firmware auf das USB-Gerät. |
| `./scripts/monitor.sh` | Öffnet den seriellen Monitor für die vorhandene ELF-Datei. |

Standardmäßig verwenden die Skripte den Workspace
`~/git/zephyrproject`, das Virtual Environment `~/.venvs/zephyr` und
`/dev/ttyACM0`. Sie setzen dessen `bin`-Verzeichnis selbst auf `PATH`; ein
vorheriges `source ~/.venvs/zephyr/bin/activate` ist daher nicht erforderlich.
Bei abweichender Einrichtung können diese Werte ohne Änderung am Repository
überschrieben werden:

```bash
ZEPHYR_WORKSPACE=/pfad/zu/zephyrproject \
WEST_BIN=/pfad/zu/west \
ESP_DEVICE=/dev/ttyACM1 \
./scripts/flash.sh
```

Die Skripte laden oder installieren keine Abhängigkeiten und verändern den
Zephyr-Workspace nicht, abgesehen von den normalen CMake- und Build-Caches.

## Onboard-WS2812

### Ziel und Schnittstelle

Der ESP32-S3-Zero besitzt eine einzelne WS2812-RGB-LED an GPIO21. Die
Komponente `RgbLed` kapselt den Zugriff darauf und bietet eine kleine,
hardwareunabhängige C++-Schnittstelle:

```text
RgbLed::initialize()       prüft den über Devicetree beschriebenen Aktor
RgbLed::set(Color)         übergibt einen RGB-Wert an den Zephyr-Treiber
```

Beide Operationen liefern einen Zephyr-Fehlercode zurück. `RgbLed` kennt weder
die Anwendung noch Netzwerk oder Zenoh. Die Anwendung verantwortet den
Farbtest und schreibt Fehler auf die serielle Konsole.

### Hardware- und Treiberentscheidung

Die WS2812 wird über den Zephyr-Treiber `worldsemi,ws2812-pulse-io`
angesteuert. Der Treiber kodiert das WS2812-Protokoll und verwendet den
hardwareseitigen RMT-TX-Kanal 0 des ESP32-S3. Das Board-Overlay ordnet dessen
Ausgang über `RMT_OUT0_GPIO21` dem Datenpin der Onboard-LED zu.

```text
Application ── RgbLed ── led_strip API ── WS2812 pulse_io driver
                                            │
                                     ESP32-S3 RMT TX0
                                            │
                                          GPIO21
```

RMT erzeugt die zeitkritischen High-/Low-Pulse per Hardware. Deshalb verwendet
die Anwendung weder Bit-Banging noch eine blockierende Warteschleife. Die
USB-Serial/JTAG-Konsole bleibt unabhängig davon auf dem nativen USB-Port.

### Farbtest und Ablauf

Nach erfolgreicher Initialisierung setzt die Anwendung rot, grün und blau in
einem Sekundenrhythmus. Ein `k_work_delayable` plant die nächste Aktualisierung
auf der Zephyr-System-Workqueue. Dadurch bleibt `main()` nach der
Initialisierung frei und der Ablauf ist später ohne Architekturbruch durch
Commands oder Zustandsänderungen ersetzbar.

Der Test verwendet genau ein Pixel und keine Heap-Allokation. Die
WS2812-spezifische Farbfolge ist im Devicetree als RGB beschrieben; die
`RgbLed::Color`-Schnittstelle bleibt dagegen in der fachlichen RGB-Reihenfolge.

### Blink-Helligkeit

Ein periodischer Zephyr-Timer schaltet die sichtbare Helligkeit unabhängig von
der Grundfarbe zwischen `0` (aus) und
`32` von `255` (etwa 12,5 Prozent) um. Die Umschaltung erfolgt alle 500 ms.
Der Farbwechsel bleibt bei einer Sekunde.

Der Timer-Callback steuert die LED nicht direkt, weil er im Timer- bzw.
Interrupt-nahen Kontext läuft. Er ändert ausschließlich den atomar
zugreifbaren Helligkeitszustand und reicht Render-Arbeit an die System-
Workqueue weiter. Nur ein Workqueue-Handler darf `RgbLed::set()` aufrufen.

```text
color_work (1 s) ──> Grundfarbe ändern ──────┐
                                              ├──> render_work ──> RgbLed::set()
blink_timer (500 ms) ──> Helligkeit 0 ↔ 32 ──┘
```

`render_work` kombiniert Grundfarbe und Helligkeit: Bei `0` schreibt er
Schwarz, ansonsten skaliert er jede RGB-Komponente mit dem Helligkeitswert.
Da Farb- und Render-Work in derselben Workqueue laufen, greifen sie
serialisiert auf die Grundfarbe zu. Der Timer teilt nur den atomaren
Helligkeitszustand mit dem Render-Work. Dadurch gibt es weder blockierende
Wartezeiten noch konkurrierende Hardwarezugriffe.

Der LED-Test wechselt bei jeder Grundfarbe im Halbsekundenrhythmus zwischen aus
und gedimmt. Der serielle Monitor darf dabei keine wiederkehrenden
Fehlerdiagnosen ausgeben.

## WLAN-Inbetriebnahme

### Ziel und Abgrenzung

Die nächste Stufe stellt eine WLAN-Verbindung im Station-Modus her, bezieht
per DHCP eine IPv4-Adresse und macht Verbindungszustand sowie Adresse seriell
sichtbar. Zenoh, ein Konfigurationsportal, Scans und eine ausgereifte
Reconnect-Strategie sind noch nicht Teil dieser Stufe.

### Hardwarebeschreibung

Der ESP32-S3 enthält das WLAN-Radio als interne Peripherie; dafür sind auf dem
ESP32-S3-Zero keine zusätzlichen GPIOs oder Pinmux-Einstellungen erforderlich.
Das ausgewählte Zephyr-Basisboard aktiviert den Knoten `&wifi` bereits. Das
lokale Board-Overlay wiederholt `status = "okay"` als expliziten Vertrag der
Anwendung mit der verwendeten Hardware.

SSID und Passwort gehören weder in den Devicetree noch in versionierte
`prj.conf`-Dateien. `wifi.conf.example` ist versioniert und zeigt die zwei
Kconfig-Werte. Vor dem ersten Verbindungsversuch wird sie nach `wifi.conf`
kopiert und lokal ausgefüllt. `wifi.conf` ist von Git ausgeschlossen;
`scripts/build.sh` übergibt sie bei Existenz als zusätzliche Zephyr-
Konfigurationsdatei. Ohne diese Datei baut die Firmware mit leeren
Platzhalterwerten, überspringt den Verbindungsversuch aber mit einer klaren
seriellen Diagnose.

### Schnittstelle und Ablauf

`Connectivity` unter `src/connectivity/` kapselt die Zephyr-Netzwerk- und
WLAN-APIs. `main()` initialisiert sie nur; `net_mgmt` startet die WPA2-PSK-
Verbindung asynchron. Nach einem erfolgreichen Verbindungsereignis startet
die Komponente DHCPv4 selbst. Dadurch sind die beiden Zustände in der Konsole
getrennt sichtbar: erst die Anmeldung am Access Point, danach die von DHCP
zugewiesene IPv4-Adresse. `CONFIG_WIFI_STA_AUTO_DHCPV4=n` verhindert dabei,
dass der ESP32-Treiber diese Schritte wieder zusammenfasst. Die Komponente
veröffentlicht zunächst keine fachlichen Nachrichten und steuert die LED nicht.

Zephyr führt WLAN- und IPv4-Ereignisse in unterschiedlichen Netzwerk-Layern.
`Connectivity` registriert deshalb getrennte `net_mgmt`-Callbacks: einen für
WLAN-Verbindungs- und Trennereignisse sowie einen für die IPv4-Adresse. Diese
Ereignismasken dürfen nicht zu einer gemeinsamen Maske verodert werden.

### Ergebnis der Inbetriebnahme

Die Verbindung wurde auf der Zielhardware erfolgreich verifiziert: Der
ESP32-S3 meldete das Station-Ereignis `WIFI_EVENT_STA_CONNECTED` und erhielt
anschließend eine IPv4-Adresse vom DHCP-Server.

Die Diagnose verlief in zwei Schritten. Zunächst wurde das Zephyr-Logging für
den Wi-Fi-Treiber aktiviert. Es bestätigte mit `Wi-Fi event: 4` die Anmeldung
am Access Point; SSID, WPA2-PSK und 2,4-GHz-Verbindung waren damit bestätigt.
Unsere Anwendung meldete diesen Erfolg zunächst dennoch nicht, weil WLAN- und
IPv4-Ereignisse in einer gemeinsamen `net_mgmt`-Maske zusammengefasst waren.
Getrennte Callbacks für die beiden Netzwerk-Layer beheben das.

Das Debug-Logging ist danach wieder deaktiviert
(`CONFIG_WIFI_LOG_LEVEL_DBG=n`). `CONFIG_LOG=y` bleibt als Infrastruktur
eingeschaltet, erzeugt ohne aktivierte Modul-Logstufe aber keine Wi-Fi-
Debugzeilen. Für eine spätere Fehlersuche kann `CONFIG_WIFI_LOG_LEVEL_DBG=y`
temporär in `prj.conf` gesetzt werden; danach ist ein Neubau nötig.

Die erste Implementierung unterstützt bewusst nur WPA2-PSK. WPA3,
Enterprise-WLAN, Konfigurationsportal und Wiederverbindung bleiben spätere
Entscheidungen.

### Lokale Zugangsdaten

Vor dem Hardwaretest wird die Beispielkonfiguration lokal kopiert und
ausgefüllt:

```bash
cp wifi.conf.example wifi.conf
```

`wifi.conf` darf nicht gestagt, committed oder in Chats geteilt werden. Nach
dem Ausfüllen reicht der normale Build-Befehl; das Build-Skript erkennt die
Datei automatisch.

### Verifikation

Der normale Ablauf bleibt lokal im Anwendungs-Repository:

```bash
./scripts/build.sh
./scripts/flash.sh
./scripts/monitor.sh
```

Ohne `wifi.conf` bleiben die LED-Diagnosen sichtbar und die Konsole meldet
fehlende oder ungültige WLAN-Zugangsdaten. Mit gültigen Zugangsdaten werden
zusätzlich eine angeforderte WLAN-Verbindung, die erfolgreiche Anmeldung am
Access Point und danach die von DHCP zugewiesene IPv4-Adresse erwartet. Bei
einem Hardware-Reset muss die WSL-USB-Anbindung weiterhin aktiv sein; während
der Entwicklung wird dafür `usbipd --auto-attach` verwendet.

## Zenoh: erste Verbindungsprobe

### Ziel und Abgrenzung

Der ESP32-S3 wird als Zenoh-Client ausgeführt und verbindet sich über TCP mit
einem Zenoh-Router auf dem Windows-Host. Diese Stufe weist ausschließlich
die Transport- und Session-Verbindung nach. Sie enthält noch keine Key
Expression, keine Publish-/Subscribe- oder Query-/Reply-Operation und keine
LED-Steuerung über Zenoh.

### Schnittstelle und Ablauf

`Connectivity` meldet die erfolgreiche IPv4-Konfiguration über einen kleinen
Callback mit Kontextzeiger an die Anwendung. Diese übergibt das Ereignis an
`ZenohClient::start()`. `ZenohClient` reiht den eigentlichen Aufruf von
`z_open()` in eine Zephyr-Workqueue ein: Der Netzwerk-Callback und die
LED-Abläufe bleiben dadurch nicht blockiert. zenoh-pico startet bei der
aktivierten Multithread-Konfiguration seine Hintergrundaufgaben nach einer
erfolgreichen Session selbst.

Der Router-Locator ist ein lokales Kconfig-Fragment `zenoh.conf`, da die
Windows-LAN-Adresse installationsabhängig ist. `zenoh.conf.example` ist
versioniert und erwartet einen expliziten TCP-Locator, beispielsweise
`tcp/192.168.1.42:7447`; die echte Datei `zenoh.conf` ist von Git ausgeschlossen.
`scripts/build.sh` übergibt vorhandene `wifi.conf` und `zenoh.conf` gemeinsam
als zusätzliche Zephyr-Konfiguration. Der Windows-Router muss auf dieser
Adresse und TCP-Port 7447 lauschen; WSL dient nur zum Bauen, Flashen und für
den seriellen Monitor.

### Stand der Abhängigkeitsprüfung

`modules/lib/zenoh-pico` ist auf Release `1.10.1` mit Commit
`e1ab223a28aaebb5dec1e70d98eab152332f777a` gepinnt. Es ist als Gitlink
vorgesehen; `.gitmodules` dokumentiert Quelle und Release-Zweig. Gegenüber dem
zunächst untersuchten 1.9.0 enthält 1.10.1 die zuvor fehlenden Runtime-Quellen
bereits.

Vor jedem Build wendet `scripts/prepare_zenoh_pico.sh` drei idempotente Patches
an:

- `0001-zephyr-version-header.patch` ist der unveränderte Inhalt des offenen
  Upstream-PRs [#1310](https://github.com/eclipse-zenoh/zenoh-pico/pull/1310).
  Er verwendet auf aktuellen Zephyr-Versionen `zephyr/version.h`, behält aber
  den alten Header als Rückfall bei.
- `0002-zephyr-generate-config.patch` ist unser lokaler Modul-Patch. Er erzeugt
  die von zenoh-pico benötigte `config.h` im Build-Verzeichnis und überträgt die
  vorhandenen Zephyr-Kconfig-Features in diese Konfiguration. Ohne ihn wird
  zenoh-picos Top-Level-CMake nicht ausgeführt und die erzeugte Headerdatei
  fehlt.
- `0003-zephyr-pthread-attribute-lifetime.patch` ist ein lokaler
  Kompatibilitäts-Patch für Zephyrs POSIX-Threads. zenoh-pico verwendet einen
  statisch reservierten Stack für seinen Hintergrund-Executor. Ein sofortiges
  `pthread_attr_destroy()` nach `pthread_create()` versucht unter Zephyr, diesen
  noch laufenden Stack freizugeben und erzeugt die Diagnose `tid ... is in use`.
  Der Patch lässt das nur lokale Attributobjekt nach erfolgreicher Erstellung
  verfallen; Zephyr verwaltet die kopierten Thread-Attribute beim Beenden des
  Threads. Damit bleibt der statische Stack bis zum Thread-Ende gültig.

Die Patches liegen versioniert unter `patches/zenoh-pico/`; das geklonte Modul
wird durch ihre Anwendung absichtlich lokal verändert, sein Commit bleibt dabei
unverändert. `prj.conf` aktiviert die Bibliothek, den Zephyr-TCP-Stack, ihr
benötigtes Threading und das dafür erforderliche Zephyr-POSIX-Profil. Die
Standardpools von Zephyr umfassen nur fünf POSIX-Mutexe und sind für eine
zenoh-pico-Session zu klein. Deshalb reservieren wir bewusst acht POSIX-Threads,
16 Mutexe und acht Condition Variables als statische Obergrenzen. Sie vermeiden
Heap-Allokation zur Laufzeit und werden bei wachsendem Funktionsumfang erneut
gemessen statt pauschal weiter erhöht.

### Update-Prüfung

Bei einem neuen zenoh-pico Release wird zuerst der aktuell angewendete Patch
zurückgesetzt (`git -C modules/lib/zenoh-pico restore .`) und das Modul auf den
neuen, bewusst gewählten Tag gesetzt. Anschließend prüft
`./scripts/check_zenoh_pico_patches.sh` jeden Patch:

- `included upstream`: Der neue Release enthält den Fix; die lokale Patchdatei
  wird nach Prüfung entfernt und die Dokumentation angepasst.
- `still required`: Der Patch lässt sich weiterhin anwenden und bleibt nötig.
- `no longer applies cleanly`: Der neue Upstream-Kontext hat sich geändert;
  der Patch wird vor einem Build fachlich überprüft und angepasst, nicht blind
  übernommen.

Nach der Entscheidung wird der erwartete Commit in
`scripts/prepare_zenoh_pico.sh`, `.gitmodules` und diesem Abschnitt gemeinsam
aktualisiert. Das verhindert unbemerkte Änderungen durch einen wandernden
Branch und macht den Upstream-Status pro Release nachvollziehbar.

### Verifikation

Auf dem Windows-Host läuft ein erreichbarer Zenoh-Router mit TCP-Listener, zum
Beispiel `zenohd -l tcp/0.0.0.0:7447`; die Windows-Firewall erlaubt den Port im
privaten Netzwerk. Nach dem Flashen meldet der serielle Monitor nach DHCP den
verwendeten Locator und entweder die erfolgreich geöffnete Zenoh-Session oder
einen eindeutigen Fehlercode. Erst nach diesem Nachweis werden Key Expressions
und die LED-Schnittstelle festgelegt.

## Dilbert: interaktives Zenoh-Werkzeug

### Ziel und Architektur

Dilbert ist das plattformübergreifende, interaktive Werkzeug für manuelle
Zenoh-Experimente und einfache Bedienung. Es liegt als eigenständiger Ordner
`dilbert/` im Projektstamm und besteht aus einem versionierten Jupyter-Notebook.
Der Name folgt der im Projekt etablierten Bezeichnung für kleine,
plattformübergreifende Bedienwerkzeuge. Das Notebook läuft in einer eigenen
Python-Umgebung unter WSL; sein Frontend wird im Browser auf Windows bedient.
Solange Dilbert das einzige Host-Werkzeug ist, bleibt die Projektstruktur
flach. Erst bei weiteren Werkzeugen wird eine gemeinsame Struktur unter
`tools/` eingeführt.

```text
Browser auf Windows → JupyterLab in WSL → Zenoh TCP → zenohd auf Windows → ESP32-S3
```

Dilbert verwendet `eclipse-zenoh` als direkten Zenoh-Client und verbindet sich
über den expliziten TCP-Locator mit dem bestehenden Windows-Router. Damit
bleiben Firmware und Werkzeug gleichwertige Zenoh-Teilnehmer; weder ein
eigener Webserver noch das Zenoh-REST-Plugin oder eine WebSocket-Bridge gehören
zu dieser Ausbaustufe.

Das Notebook-Format erlaubt, jeden Versuch mit Erklärung, Python-Code und
sichtbarem Ergebnis festzuhalten. Das `.ipynb` und sein Python-Code sind
plattformübergreifend versionierbar. Die Virtual Environment selbst ist
plattformabhängig und wird nicht versioniert; sie bleibt getrennt von der
Zephyr-Toolchain, damit Python-Abhängigkeiten für Firmware-Build und Dilbert
einander nicht beeinflussen. `dilbert/requirements.txt` beschreibt JupyterLab
und `eclipse-zenoh`; `config.example.py` ist die versionierte Vorlage für den
lokalen Router-Locator. Die daraus kopierte Datei `config.py` bleibt lokal und
ist von Git ausgeschlossen.

### Erste Schnittstelle und Ablauf

Der erste Dilbert-Schritt öffnet eine explizit konfigurierte Session und
deklariert einen Publisher für `nachtlicht/led/color/next`. Er sendet als
Text-Payload `next`; die erste Firmware-Version prüft nur das Eintreffen einer
gültigen Nachricht. Jede Nachricht fordert genau den Wechsel zur nächsten
Farbe der bestehenden Testreihe an. Der Blink-/Helligkeits-Timer auf dem
ESP32 bleibt dabei unverändert aktiv.

`ZenohClient` deklariert nach dem erfolgreichen Öffnen der Session einen
Subscriber für denselben Key. Er meldet jede empfangene Nachricht über einen
Callback mit Kontextzeiger an die Anwendung. Dieser Callback darf die WS2812
nicht direkt aus dem Zenoh-Hintergrundthread ansteuern: Er reicht ausschließlich
den bestehenden `color_work` weiter. Der Work-Handler setzt die nächste Farbe
und reiht danach `render_work` für den Hardwarezugriff ein. Dadurch bleiben
Zenoh-Verarbeitung, Timer und Hardwarezugriff voneinander entkoppelt.

Beim Boot setzt der Work-Handler einmalig die erste Testfarbe. Anschließend
wechseln die Farben ausschließlich auf Dilbert-Anforderung; die bisherige
periodische Farb-Workqueue entfällt. Die serielle Diagnose meldet die
erfolgreiche Subscriber-Deklaration und jede empfangene Farbwechsel-Anforderung.
`CONFIG_ZENOH_PICO_SUBSCRIPTION=y` aktiviert dafür gezielt die
Subscriber-API von zenoh-pico; Publisher, Query und Queryable bleiben in der
Firmware weiterhin deaktiviert, solange sie nicht benötigt werden.

### Abgrenzung

Eine Browser-App mit Zenoh REST oder WebSocket-Bridge, eine native
Windows-Anwendung und eine Rust-Implementierung bleiben sinnvolle spätere
Alternativen. Sie sind nicht Teil des ersten End-to-End-Nachweises. Jupyter
auf dem ESP32 selbst wäre eine MicroPython-Umgebung und würde die aktuelle
Zephyr/C++-Firmware ersetzen; Dilbert läuft deshalb ausschließlich auf dem
Host.

## Entwicklungsreihenfolge

1. Zephyr-Projekt und Build/Flash für das ESP32-S3-Zero verifizieren.
2. Onboard-WS2812 über `RgbLed` ansteuern.
3. Netzwerkverbindung herstellen und diagnostizieren.
4. zenoh-pico integrieren und eine TCP-Session Host ↔ Controller nachweisen.
5. Mit Dilbert eine Zenoh-Nachricht an den Controller senden.
6. LED über Zenoh steuern und State publizieren.
7. Kommunikationsmuster, Lifecycle und Reconnect untersuchen.
8. Erst danach Home Assistant oder weitere Hardware evaluieren.

## Offene Entscheidungen

- Zephyr-Board-Target und Board-spezifisches Devicetree-Overlay
- Einbindung, Speicherbedarf und Threading-Modell von zenoh-pico
- Key Expressions, Payload-Schema, Versionsstrategie und Fehlervertrag
- Teststrategie auf Host und Hardware

## Referenzen

- [ESP32-S3 Datasheet (PDF)](https://documentation.espressif.com/esp32_s3_datasheet_en.pdf)
  – Pinbelegung, elektrische Daten und Überblick über die Peripherie.
- [ESP32-S3 Technical Reference Manual (PDF)](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
  – Details zur RMT-Peripherie und GPIO-Matrix.
- [Espressif: Technical Documents](https://espressif.com/en/support/download/documents)
  – zentrale Download-Seite für die jeweils aktuellen Dokumentversionen.

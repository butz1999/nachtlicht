# Software-Architektur

## Zweck und Status

Diese Architektur beschreibt den minimalen Zephyr-/Zenoh-Demonstrator. Sie ist
eine Arbeitsgrundlage und wird mit jeder Lernstufe präzisiert. Sie schreibt
weder ein allgemeines Framework noch ein finales Produktprotokoll vor.

Die erste lauffähige Stufe steuert ausschließlich die Onboard-WS2812. Netzwerk,
Zenoh und Home Assistant folgen erst danach.

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

## Diagnose und Debugging

Serielle Zephyr-Logs sind der Standard für die erste Inbetriebnahme und für
Laufzeitdiagnosen. Sie werden vor einem Hardware-Debugger eingerichtet.

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

## Entwicklungsreihenfolge

1. Zephyr-Projekt und Build/Flash für das ESP32-S3-Zero verifizieren.
2. Onboard-WS2812 über `RgbLed` ansteuern.
3. Netzwerkverbindung herstellen und diagnostizieren.
4. zenoh-pico integrieren; Kommunikation Host ↔ Controller nachweisen.
5. LED über Zenoh steuern und State publizieren.
6. Kommunikationsmuster, Lifecycle und Reconnect untersuchen.
7. Erst danach Home Assistant oder weitere Hardware evaluieren.

## Offene Entscheidungen

- Zephyr-Board-Target und Board-spezifisches Devicetree-Overlay
- Treiber- und Konfiguration der WS2812 unter Zephyr
- Wi-Fi-Konfiguration und Umgang mit Zugangsdaten
- Einbindung, Speicherbedarf und Threading-Modell von zenoh-pico
- Key Expressions, Payload-Schema, Versionsstrategie und Fehlervertrag
- Teststrategie auf Host und Hardware

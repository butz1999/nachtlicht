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

# Project Context – Zephyr + Zenoh Demonstrator

## Ziel

Dieses Projekt ist primär ein **Lernprojekt für Zephyr RTOS und Zenoh**.

Der konkrete Hardware-Demonstrator soll absichtlich minimal bleiben. Der Schwerpunkt liegt auf:

- Zephyr RTOS kennenlernen
- moderne C++-Entwicklung auf einem ESP32 mit Zephyr
- Zenoh praktisch verstehen
- Kommunikationsmuster zwischen Host und Embedded Controller untersuchen
- später optional Home Assistant als komfortablen Host/Client anbinden

Das Hardware-Projekt selbst ist **nicht das eigentliche Ziel**. Zusätzliche Hardware soll nur dann ergänzt werden, wenn sie beim Lernen von Zephyr oder Zenoh einen sinnvollen Anwendungsfall liefert.

---

## Geplanter Demonstrator

Start:

```text
Linux / WSL
     │
   Zenoh
     │
 Wi-Fi / IP
     │
ESP32 + Zephyr + C++
     │
 Onboard RGB LED / NeoPixel
```

Später eventuell:

```text
ESP32
 ├── RGB LED / NeoPixel
 ├── LED Strip
 ├── Temperatur-/Feuchtesensor
 └── Radar-Präsenzsensor
```

Daraus könnte ein einfaches bewegungsabhängiges Nachtlicht entstehen.

Die Anwendung soll aber bewusst einfach bleiben.

---

## Geplanter Entwicklungsweg

Nicht alles gleichzeitig integrieren.

```text
1. ESP32 + Zephyr Hello World
           ↓
2. Onboard NeoPixel ansteuern
           ↓
3. Netzwerkverbindung
           ↓
4. zenoh-pico integrieren
           ↓
5. Zenoh PC ↔ ESP32
           ↓
6. LED über Zenoh steuern
           ↓
7. Pub/Sub, Query, State, Events,
   Discovery und Lifecycle untersuchen
           ↓
8. Home Assistant anbinden
           ↓
9. Optional weitere Sensoren/Aktoren
```

Home Assistant soll bewusst **erst später** hinzukommen, damit bei den ersten Schritten nicht gleichzeitig Zephyr, Netzwerk, Zenoh und HA debuggt werden müssen.

---

## Kommunikationskonzepte

Das Projekt soll insbesondere folgende Konzepte praktisch untersuchen:

- Publish / Subscribe
- Query / Reply
- Commands
- State
- Events
- Discovery
- Reconnect
- Lifecycle
- Fehlerbehandlung
- Datenmodell / Key Expressions
- synchrone vs. asynchrone Operationen

Beispiel:

```text
PC / Home Assistant
       │
       │ command
       ▼
     ESP32
       │
       ├── LED setzen
       │
       └── State publizieren
```

Mögliche Zenoh Keys könnten beispielsweise sein:

```text
demo/light/set
demo/light/state
demo/presence/state
demo/environment/temperature
demo/environment/humidity
```

Diese Namen sind noch **keine festgelegte Architektur**.

---

## State vs. Event

Eine wichtige konzeptionelle Trennung:

**State**

> Was gilt aktuell?

Beispiele:

```text
light/state = on
presence/state = true
temperature = 22.4
```

**Event**

> Was ist passiert?

Beispiele:

```text
presence_detected
controller_booted
operation_completed
```

Die genaue Modellierung soll während des Projekts untersucht werden.

---

## Bezug zu realen Embedded-Systemen

Neben dem persönlichen Lernziel soll das Projekt Erkenntnisse liefern, die möglicherweise für zukünftige Embedded-Architekturen bei HSE AG interessant sind.

Typische reale Systeme bestehen aus:

```text
Embedded Linux Host
       │
       │ Kommunikation
       ▼
Main Controller / µC
       │
       │ CAN
       ▼
weitere Controller
```

Historisch erfolgt Host ↔ Main-Firmware häufig über USB-Serial und ein proprietäres Protokoll.

Das bisherige Kommunikationsmodell kennt ungefähr:

### Synchrone Commands

Der Host sendet einen Command und blockiert normalerweise bis zur Antwort bzw. einem Timeout.

Typischerweise eher Low-Level-Funktionen, beispielsweise GPIO lesen/schalten.

### Asynchrone Commands

Prinzip:

```text
CMD → ACK → ...Ausführung... → DONE
```

Typischerweise höherwertige Operationen wie:

```text
Init
Move
```

### Events

Events waren hauptsächlich flüchtig.

Historisch gab es Konzepte für:

- zyklische Übertragung
- On-Change
- Kombination aus beiden

### Fehler

Commands besitzen einen Error-Code.

### Datenmodell

Es gibt bisher keine allgemeine Serialisierungsvorgabe.

Das Datenschema wurde üblicherweise direkt in der Protokollspezifikation dokumentiert.

Protobuf oder ähnliche Technologien sind daher **keine gesetzte Voraussetzung** und sollen nur verwendet werden, wenn ein konkreter Mehrwert entsteht.

### Bandbreite

Die Anforderungen sind gering.

Beispielsweise wurden Achspositionen bisher ungefähr mit 1 Hz übertragen. Auch etwa 10 Hz wären bereits komfortabel.

Hochfrequente Datenströme sind selten.

### Echtzeit

Zwischen Linux-Host und Mikrocontroller besteht **keine Hard-Realtime-Anforderung**.

Echtzeitkritische Funktionen gehören in den Mikrocontroller.

Die Host-Kommunikation transportiert beispielsweise:

- Commands
- Parameter
- State
- Events
- Diagnosedaten

### Topologie

Es gibt keine zwingende zukünftige Topologie.

Mit Ethernet bzw. perspektivisch Single Pair Ethernet könnten Controller direkt IP-fähig werden.

Dadurch könnte eine heutige Struktur

```text
Linux
  │
Main Controller
  │
 CAN
  ├── Controller A
  ├── Controller B
  └── Controller C
```

langfristig auch eher so aussehen:

```text
             Ethernet / IP

Linux ─────────┬─────────┬─────────┐
               │         │         │
          Controller A   B         C
```

Damit wäre ein Main Controller nicht mehr zwangsläufig Kommunikations-Gateway.

**Single Pair Ethernet ist jedoch nicht Bestandteil dieses Demonstrators.**

---

## Zenoh als Untersuchungsgegenstand

Zenoh ist interessant, weil es mehrere Kommunikationsmuster kombiniert:

```text
Pub/Sub
Query/Reply
Discovery
Routing
```

Mögliche Abbildung des bestehenden Modells:

```text
bestehend                 mögliche Zenoh-Abbildung

Synchroner CMD        →   Query / Reply

Asynchroner CMD       →   Query/Reply für CMD/ACK
                         +
                         Publish für DONE/Status

Event                 →   Publish / Subscribe

State                 →   Publish / Query
```

Diese Zuordnung ist eine **Arbeitshypothese und keine festgelegte Architektur**.

Ein wesentliches Ziel des Projekts ist gerade herauszufinden, welche Abbildungen sinnvoll sind.

---

## MQTT, Matter und andere Protokolle

Zenoh soll praktisch gelernt werden.

Andere Protokolle dienen zunächst nur zum konzeptionellen Vergleich.

### MQTT

- primär Publish/Subscribe
- normalerweise zentraler Broker
- sehr etabliert
- sehr gute Home-Assistant-Unterstützung
- Request/Response muss typischerweise auf Anwendungsebene modelliert werden

### Matter

Matter ist nicht direkt mit Zenoh vergleichbar.

Matter definiert wesentlich mehr:

- Device Model
- Commissioning
- Security
- Discovery
- standardisierte Gerätefunktionen
- Interoperabilität

Matter kann deshalb als Inspiration dienen, welche Probleme **oberhalb einer Kommunikationsschicht** noch gelöst werden müssen.

### Weitere interessante Vergleichspunkte

Nur konzeptionell:

- DDS
- CoAP
- OPC UA

Es ist ausdrücklich **nicht geplant, alle diese Protokolle zu implementieren**.

---

## Lifecycle / Reconnect

Ein besonders interessanter zukünftiger Anwendungsfall:

```text
Controller läuft
      ↓
Controller Reset
      ↓
Boot
      ↓
Netzwerk
      ↓
Zenoh verfügbar
      ↓
Host erkennt Controller wieder
      ↓
State synchronisieren
      ↓
betriebsbereit
```

Dabei könnten später beispielsweise folgende Informationen relevant werden:

- Controller Identity
- Firmware-Version
- API-/Protokoll-Version
- Boot-ID
- aktueller State
- Konfiguration

Lifecycle ist bisher noch nicht spezifiziert und soll während des Projekts untersucht werden.

---

## Entwicklungsprinzipien

### Keep it simple

Der Demonstrator soll nicht unnötig komplex werden.

Insbesondere zunächst vermeiden:

- großes eigenes Framework
- unnötige Abstraktionsschichten
- Dependency-Injection-Architektur
- komplexe Hardware
- unnötige Serialisierung
- frühzeitige Optimierung

Erst abstrahieren, wenn konkrete Anforderungen entstehen.

Eine mögliche anfängliche C++-Struktur könnte beispielsweise lediglich sein:

```cpp
class RgbLed;
class ZenohClient;
class Application;
```

Auch dies ist noch keine Vorgabe.

### Kommunikation vor Hardware

Wenn eine Erweiterung hauptsächlich Hardwarearbeit erzeugt, aber wenig über Zephyr oder Zenoh lehrt, hat sie geringe Priorität.

### Lernen vor Produktarchitektur

Das Projekt soll nicht sofort eine zukünftige HSE-Kommunikationsarchitektur definieren.

Primäres Ziel:

> Zephyr lernen und Zenoh wirklich verstehen.

Sekundäres Ziel:

> Erkenntnisse gewinnen, die für eine zukünftige standardisierte Host↔µC-Kommunikation nützlich sein könnten.

---

## Nächster Schritt

Das GitHub-Repository wird unter WSL mit VS Code bearbeitet.

Als Nächstes:

1. konkretes ESP32-Board identifizieren
2. Entwicklungsumgebung prüfen
3. Zephyr-Projekt aufsetzen
4. minimales Hello World bauen und flashen
5. danach Onboard-NeoPixel ansteuern

Erst anschließend beginnen wir mit Netzwerk und Zenoh.

**Noch kein umfangreiches Projektdokument oder Architekturpapier erstellen.**

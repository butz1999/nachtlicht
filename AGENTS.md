# Zusammenarbeit

## Arbeitsweise

- Dokumentation zuerst: Bevor Architekturentscheidungen oder Implementierungen getroffen werden, halten wir Ziel, Annahmen, Schnittstellen und offene Fragen kurz in Markdown fest.
- Dokumentation bleibt knapp, konkret und aktuell. Markdown-Dateien sind die maßgebliche Projektdokumentation.
- Dokumentation und Code werden stets synchron gehalten. Jede fachlich oder technisch relevante Codeänderung aktualisiert im selben Arbeitsschritt die betroffene Dokumentation; umgekehrt wird dokumentierter, implementierter Stand nicht wissentlich von der Codebasis abweichen gelassen.
- Architektur vor Implementierung: Wir klären die Architektur gemeinsam, bevor wir Code schreiben. Entscheidungen werden erst danach schrittweise implementiert und gemeinsam überprüft.
- **Lernfokus:** Zephyr und Zenoh sind für den Benutzer noch neu. Erkläre bei der gemeinsamen Entwicklung relevante Konzepte, Entscheidungen, Zusammenhänge und Alternativen verständlich und in angemessener Tiefe.
- Commits erstellt ausschließlich der Benutzer. Erstelle, amendiere oder pushe keine Git-Commits, sofern nicht ausdrücklich darum gebeten wird.

## Technischer Fokus

Dieses Projekt ist ein Zephyr-RTOS- und Zenoh-Demonstrator im ESP32-/ESP32-S3-Umfeld. Schreibe dafür effizienten, stabilen und gut dokumentierten C++-Code.

Die endgültige Projektstruktur ist noch offen. Anwendungscode gehört grundsätzlich nach `./src`.

## Programmier-Richtlinien & Code-Stil

- **Sprache:** C++17 im Zephyr-Umfeld.
- **Kommentare:** Kommentare im Source-Code werden auf Englisch geschrieben.
- **Modularität:** Halte die Module und Vorgaben aus `doc/sw/software.md` ein.
- **Asynchroner Code:** Vermeide blockierende Ablaufsteuerung. Bevorzuge deterministische und testbare Zephyr-Mechanismen wie Workqueues, Events und Message Queues.
- **Hardwarebezug:** Berücksichtige die jeweils dokumentierte Zielhardware und ihre Schnittstellen.
- **Zephyr-Konfiguration:** Beschreibe Hardware und Pins primär über Devicetree und Overlays; verwende `prj.conf` und Kconfig für die Konfiguration. Benannte C++-Konstanten sind nur für fachliche Signale oder Werte vorgesehen, die nicht zum Devicetree gehören.
- **Nebenläufigkeit:** ISR-Routinen bleiben kurz und dürfen weder blockieren noch I2C- oder andere potenziell blockierende Arbeiten ausführen. Übergib solche Arbeiten an geeignete Zephyr-Mechanismen.
- **Fehlerbehandlung:** Prüfe besonders Schnittstellen-, Kommunikations-, Konfigurations- und Initialisierungsfehler. Mache Fehler über klare Rückgabemodelle und serielle Diagnose sichtbar.
- **Speicher-Management:** Bevorzuge Stack-Allokation. Vermeide Heap-Allokation mit `new`/`malloc` sowie `delete`, um Speicherfragmentierung zu verhindern. Informiere den Benutzer, falls Heap-Allokation unvermeidbar ist.
- **Abhängigkeiten:** Füge externe Bibliotheken nur nach gemeinsamer Architekturentscheidung hinzu und dokumentiere ihren Zweck sowie ihre Integration kurz.
- **Modernes C++:**
  - Verwende `auto` für Iteratoren und komplexe Typen.
  - Verwende `nullptr` statt `NULL` oder `0`.
  - Verwende Typ-Aliase mit `using` statt `typedef`.
  - Verwende `enum class` für Zustände statt unscoped Enums.
  - Bevorzuge `std::array` oder `std::vector` gegenüber rohen C-Arrays; bei `std::vector` den Heap-Einsatz beachten.
  - Verwende `constexpr` für Compile-Time-Konstanten, insbesondere bei Pin-Definitionen.
- **Namespaces:** Komponentenordner unter `src/` bilden die primären Namespaces. Verschachtelte Namespaces nur verwenden, wenn innerhalb einer Komponente eine echte fachliche Unterstruktur besteht.
- **Dokumentationspflege:** Aktualisiere bei Architektur-, Schnittstellen-, Zenoh-Key-Expression- oder Hardwareänderungen die passende Dokumentation unter `doc/`.

## Verifikation

- Behaupte nicht, dass Code kompiliert, getestet oder auf Hardware lauffähig ist, ohne dies tatsächlich geprüft zu haben.
- Führe nach jeder Implementierung als letzten Bearbeitungsschritt `clang-format -i` auf allen geänderten C/C++-Quell- und Headerdateien aus. Verwende die Konfiguration aus `.clang-format` im Projektstamm. Berichte, falls `clang-format` nicht verfügbar ist.
- Führe, soweit möglich, eine passende Verifikation aus: mindestens einen Build oder Tests.
- Falls Verifikation unter WSL wegen Tooling-Problemen oder fehlender Hardware nicht möglich ist, benenne dies ausdrücklich.
- Eine Implementierung ist erst abgeschlossen, wenn Formatierung und die mögliche Verifikation ausgeführt sowie Änderungen oder Einschränkungen dokumentiert wurden.

## Git-Arbeitsregeln

- Codex darf Änderungen vorbereiten, prüfen und erklären.
- Ausschließlich der Benutzer führt Staging mit `git add` sowie Commits mit `git commit` aus.
- Codex nennt bei Bedarf passende Git-Befehle, führt sie jedoch nicht selbst aus.

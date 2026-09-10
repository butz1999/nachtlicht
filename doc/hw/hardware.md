# Hardware-Basis

## Entwicklungsboard

Dieses Projekt verwendet ein **Waveshare ESP32-S3-Zero** Entwicklungsboard mit
dem ESP32-S3FH4R2.

| Merkmal | Wert |
| --- | --- |
| CPU | Dual-Core Xtensa LX7, bis 240 MHz |
| Flash | 4 MB |
| PSRAM | 2 MB |
| Funk | 2,4-GHz Wi-Fi (802.11 b/g/n), Bluetooth 5 LE |
| USB | Native USB Full-Speed über USB-C; kein USB-UART-Wandler |
| Onboard-LED | WS2812 RGB LED an GPIO21 |
| BOOT | GPIO0 |
| UART0 | TX: GPIO43, RX: GPIO44 |

## Verbindliche Hardware-Vorgaben

- GPIO33 bis GPIO37 sind für das Octal-PSRAM reserviert und werden nicht für
  externe Peripherie verwendet.
- GPIO0 ist der BOOT-Pin und darf nicht durch externe Beschaltung oder die
  Anwendung beeinträchtigt werden.
- GPIO21 ist zunächst für die Onboard-WS2812 reserviert. Die LED ist der erste
  geplante Aktor des Demonstrators.
- UART0 auf GPIO43/GPIO44 bleibt bis zu einer bewussten Entscheidung für eine
  andere Funktion frei.
- Die Keramikantenne bleibt frei von metallischen oder abschirmenden Bauteilen.
- Hardware und Pins werden in Zephyr primär über Devicetree und Overlays
  beschrieben, nicht im Anwendungscode fest verdrahtet.

## Offene Entscheidungen

- Exakte Zephyr-Board- und SoC-Target-Bezeichnung im Build-System
- Devicetree-Overlay für das Waveshare-Board
- Belegung der verfügbaren GPIOs für spätere Sensoren und Aktoren
- Art der seriellen Diagnose über die native USB-Schnittstelle

## Quellen

- [Waveshare: ESP32-S3-Zero Dokumentation](https://docs.waveshare.com/ESP32-S3-Zero)
- [Waveshare: Ressourcen, Pinout und Schaltplan](https://docs.waveshare.com/ESP32-S3-Zero/Resources-And-Documents)
- [Bastelgarage: Produktseite](https://www.bastelgarage.ch/esp32-s3-zero-entwicklungsboard?search=esp32-s3%20zero)

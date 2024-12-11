# DCC Signalgenerator
DCC (Digital Command Control) Signalgenerator für Arduino Uno R3 und kompatible Geräte mit Arduino Motor-Shield zum Betrieb von digitalen Modelleisenbahnen basierend auf den Normen der https://railcommunity.de, insbesondere den Normen RCN-210, RCN-211 und RCN-212.

Umgesetzte Funktionen:
- Timing-Korrekte Generierung von DCC Paketen
- Kurze (1-127) und lange Lokadressen (1-10239)
- Funktionen 0-68
- 14, 28 und 128 Fahrstufen
- Nothalt- und Idle-Pakete

Geplant:
- Notabschaltung Gleisspeisung bei Kurzschluss
- Kontinuierlicher Rückmeldemodus über Serielle Schnittstelle zur Überwachung
  der Last
- Programmiermodus

Limitierungen:
- Mit einem Arduino Uno R3 (ATmega328) können ca. 80 Loks gleichzeitig betrieben werden (Arbeitsspeicherlimit). Dieser Wert ist aber eher theoretischer Natur, da die Arduino Motor-Shields mit L298 H-Brücke maximal 2A treiben können. Realistischer Weise entspricht das ca. 8-10 Zügen.

## Kommandoübersicht
Der DCC Generator nimmt über serielle Schnittstelle (USB) textbasierte
Kommandos zur Steuerung entgegen. Im einfachsten Fall reicht zum Testen also
ein Programm zur Ansteuerung der Seriellen Schnittstelle
(minicom, Hterm etc.). Es existiert ein ca. zwei Sekunden Timeout zwischen einzelnen Zeichen beim Lesen eines Kommandos. Beim händischen Tippen von Kommandos sollte man sich daher nicht all zu viel Zeit lassen.

Kommandos beginnen mit `!` und enden mit Zeilenvorschub.
Nach erfolgreicher Abarbeitung eines Kommandos sendet das Programm `?OK` und Zeilenvorschub.
Bei auftreten eines Fehlers `?ERR` und Zeilenvorschub.

Folgende Kommandos werden akzeptiert, dabei steht z für eine gültige Lokadresse und n für eine dem Kontext entsprechend gültige Zahl:

    !I        Versionsinformation abrufen

    !R        Reset: DCC-Reset Pakete senden,
              alle Züge deaktivieren und Geschwindigkeit sowie Funktionen zurücksetzen

    !P+       Gleisspeisung ein
    !P-       Gleisspeisung aus
    !H+       Nothalt für alle Züge ein
    !H-       Nothalt aus

    !zA+      DCC Signal Zug z aktivieren
    !zA-      DCC Signal Zug z deaktivieren

    !zC+      14 Fahrstufen-Modus für Zug z
    !zC0      14 Fahrstufen-Modus
    !zC-      28 Fahrstufen-Modus für Zug z (standard)
    !zC1      28 Fahrstufen-Modus
    !zC2      128 Fahrstufen für Zug z

    !zVn      Vorwärtsgeschwindigkeit n für Zug z setzen
    !zRn      Rückwärtsgeschwindigkeit n für Zug z setzen

    !zFn+     Funktion n für Zug z aktivieren
    !zFn-     Funktion n für Zug z deaktivieren

Im folgenden Beispiel wird die Gleisspeisung sowie die Generierung des Signals für Lokadresse 1 aktiviert, das Fahrtlicht eingeschaltet (Funktion 0) und die Geschwindigkeitsstufe 14 (vorwärts) gesetzt. Sofern alles elektrisch richtig verdrahtet ist sollte sich eine DCC konforme Lok daraufhin in Bewegung setzen.

    !P+
    !1A+
    !1F0+
    !1V14


## Paket-Scheduling
Der Generator gibt kontinuierlich ein DCC Signal aus. Sollte keine Lokadresse aktiv sein werden zur Aufrechterhaltung der Stromversorgung DCC Idle-Pakete ausgegeben. Sollten Lokadressen aktiv sein werden die Pakete für Geschwindigkeit und Funktionen 0-12 in regelmäßigen Abständen wiederholt. Dies geschieht für alle aktiven Lokadressen alternierend, also gleichberechtigt. Wenn Änderungen an den Fahrstufen oder Funktionen mittels serieller Schnittstelle (USB) eingehen, werden die daraus resultierenden DCC Pakete außer der Reihe und mehrmals wiederholt gesendet. Ziel ist, dass die Empfängerlok die Information a) so schnell wie möglich und b) möglichst zuverlässig empfängt.

Die Funktionen 0-12 jeder aktivierten Lokadresse werden kontinuierlich gesendet. Dies ist bei den Funktionen 13-68 aus Optimierungsgründen anders. Das kontinuierliche Senden dieser Funktionen für eine spezifische Lokadresse wird nur aktiviert, nachdem ein Kommando zum Setzen oder Rücksetzen für eine dieser Funktionen eingegangen ist.


## Hinweis zum Arduino Motor-Shield
In den DCC Normen ist als Gleisspannung ein Wert von (je nach Spurgröße) ca. 18V (max.) spezifiziert. Diese Spannung kann dem Motor-Shield mittels geeignetem Netzteil über die Eingangsklemmen Vin und GND zugeführt werden. ACHTUNG: Die Leitung zum Vin Port des Arduino muss unbedingt vorher gekappt werden. Bei einigen Motor-Shields existiert dafür eine Drahtbrücke. Erfolgt dies nicht führt dies zur Zerstörung des Arduino!


## Bauen und Hochladen des Programms
Es wird AVR-GCC zum Bauen benötigt. Getestet habe ich es mit GCC 11 und
GCC 13. Andere Versionen sollten ebenfalls funktionieren. Für GCC 12-13.2
muss der Hinweis im Makefile beachtet werden! Zum Bauen des Programms sollte
bei korrekt eingerichtetem ACR-GCC Toolchain ein simples `make` im
Projektverzeichnis genügen. Das Programm kann mit `make upload PORT=... BAUD=...`
auf den Arduino geladen werden. Dies setzt ein installiertes avrdude Programm
voraus. PORT und BAUD (standardmäßig 115200) müssen entsprechend der Seriellen
Schnittstelle angepasst werden.

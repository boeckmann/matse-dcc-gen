# DCC Signalgenerator
DCC (Digital Command Control) Signalgenerator für Arduino Uno R3 und
kompatible Geräte mit Arduino Motor-Shield basierend auf den Normen der
https://railcommunity.de, insbesondere der Normen RCN-210, RCN-211 und
RCN-212.

Momentan umgesetzte Funktionen:
- Timing-Korrekte Generierung von DCC Paketen
- Zugadressen 1-127 (Arbeitsspeicherlimitierung auf ca. 20 Züge gleichzeitig)
- Funktionen 0-12
- 14 und 28 Fahrstufen
- Nothalt- und Idle-Pakete

Geplant:
- lange Lokadressen
- Funktionen 13-28
- 128 Fahrstufen
- Notabschaltung Gleisspeisung bei Kurzschluss
- Kontinuierlicher Rückmeldemodus über Serielle Schnittstelle zur Überwachung
  der Last


## Kommandoübersicht
Der DCC Generator nimmt über serielle Schnittstelle (USB) textbasierte
Kommandos zur Steuerung entgegen. Im einfachsten Fall reicht zum Testen also
ein Programm zur Ansteuerung der Seriellen Schnittstelle
(minicom, Hterm etc.).

Kommandos beginnen mit `!` und enden mit Zeilenvorschub.
Nach erfolgreicher Abarbeitung eines Kommandos sendet das Gerät `?OK` und Zeilenvorschub.
Bei auftreten eines Fehlers `?ERR` und Zeilenvorschub.

Folgende Kommandos werden akzeptiert:

    !I          Gerät initialisieren und Version senden
    !R          Rücksetzbefehl (unimplementiert)

    !zzzC+      14 Fahrstufen-Modus für Zug zzz
    !zzzC-      28 Fahrstufen-Modus für Zug zzz (standard)

    !H+         Nothalt für alle Züge ein
    !H-         Nothalt aus

    !zzzA+      DCC Signal Zug zzz aktivieren
    !zzzA-      DCC Signal Zug zzz deaktivieren

    !zzzVss     Vorwärtsgeschwindigkeit ss für Zug zzz setzen
    !zzzRss     Rückwärtsgeschwindigkeit ss für Zug zzz setzen

    !zzzFff+    Funktion ff für Zug zzz aktivieren
    !zzzFff-    Funktion ff für Zug zzz deaktivieren


## Bauen und Hochladen des Programms
Es wird AVR-GCC zum Bauen benötigt. Getestet habe ich es mit GCC 11 und
GCC 13. Andere Versionen sollten ebenfalls funktionieren. Für GCC 12-13.2
muss der Hinweis im Makefile beachtet werden! Zum Bauen des Programms sollte
bei korrekt eingerichtetem ACR-GCC Toolchain ein simples `make` im
Projektverzeichnis genügen. Das Programm kann mit `make upload PORT=... BAUD=...`
auf den Arduino geladen werden. Dies setzt ein installiertes avrdude Programm
voraus. PORT und BAUD (standardmäßig 9600) müssen entsprechend der Seriellen
Schnittstelle angepasst werden.

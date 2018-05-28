# DCC Signalgenerator

## Kommandoübersicht
Der DCC Generator nimmt über serielle Schnittstelle (USB) textbasierte Kommandos zur Steuerung entgegen.

Kommandos beginnen mit `!` und enden mit Zeilenvorschub.
Nach erfolgreicher Abarbeitung eines Kommandos sendet das Gerät `?OK` und Zeilenvorschub.
Bei auftreten eines Fehlers `?ERR` und Zeilenvorschub.

Folgende Kommandos werden akzeptiert:

    !I          Gerät initialisieren und Version senden

    !H+         Nothalt für alle Züge ein
    !H-         Nothalt aus

    !zzzA+      DCC Signal Zugnummer zzz aktivieren
    !zzzA-      DCC Signal für Zugnummer zzz deaktivieren

    !zzzVss     Vorwärtsgeschwindigkeit ss für Zug zzz setzen
    !zzzRss     Rückwärtsgeschwindigkeit ss für Zug zzz setzen

    !zzzFff+    Funktion ff für Zug zzz aktivieren
    !zzzFff-    Funktion ff für Zug zzz deaktivieren

# Change Log

## 2020-08-13, (v0.2.4 bis) v0.2.6 

Display:
- Logo und Umlaute implementiert
- Stop-Taste verlässt Setup-Untermenüs
- Anzeige der vorherigen Werte im Setup
- minimaler Servowinkel einstellbar
- Reihenfolge der Boot-Meldungen optimiert

Programmlogik:
- Autokorrektur implementiert
- Kulanzwert für Autokorrektur einstellbar
- Kalibrierung der Waage verbessert


## 2020-06-15, v0.2.3

Hardware:
- Angepasst an Heltec NEW Wifi Kit 32 mit angepasster Beschaltung

Programmcode:
- nicht/wenig benutzte Funktionen und Variablen eliminiert

Display:
- Display-Ausgabe mit sprintf-Formatierung und kleinerem Font
- Play/Pause Symbol zeigt an, ob der Servo aktiv ist
- Boot-Meldung

Programmlogik:
- Servo wird beim Boot und Umschalten zwischen den Modi auf Minimum gefahren
- Umschaltung zwischen den Modi deaktiviert den Servo
- Der Automatik-Modus muss ebenfalls mit der grünen Taste aktiviert werden:
   - ohne Autostart kann die Automatik _nur_ aktiviert werden, wenn ein Glas (oder anderes Gewicht!) auf der Waage steht.
     Es kann also ein leeres oder teilweise gefülltes Glas bis zum konfigurierten Wert gefüllt werden.
   - bei Autostart wird die Automatik mit grün aktiviert und startet, wenn für 1.5 Sekunden ein leeres Glas auf der Waage steht (Anzeige START).
     Damit wird verhindert, dass ein schwankendes Gewicht den Vorgang auslöst. 
     Ändert sich in den 1.5 Sekunden das Gewicht, wird der Vorgang nicht gestartet!
  Mit der roten Taste werden beide Automatiken unterbrochen und der Servo auf Minimum gefahren (Not-Aus). 
  Auch der Autostart wird damit unterbrochen und muss mit grün neu gestartet werden!

Diese Änderungen müssen sich in der Praxis noch bewähren, z.B.:
- wird durch Nachtropfen der Autostart unterbrochen?
- soll ein halbvolles Glas über die Automatik (ohne Autostart) weiter befüllt werden?

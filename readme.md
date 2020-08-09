# HaniMandl 

Ein halbautomatischer Honig-Abfüll-Roboter.

Websites:
- Code, Infos zur Hardware, Fotos und Videos initial publiziert in der Facebook-Gruppe ["Imkerei und Technik. Eigenbau"](https://www.facebook.com/groups/139671009967454)
- Weitere Infos unter: [HaniMandl, halbautomatischer Honig-Abfüllbehälter](https://community.hiveeyes.org/t/side-project-hanimandl-halbautomatischer-honig-abfullbehalter/768)

---

# Anleitung

## Einstellungen im Arduino-Code

Das Verhalten des Codes wird über mehrere defines gesteuert:

```
#define HARDWARE_LEVEL 2
  1 für originales pinout Layout mit Schalter auf Pin 19/22/21
  2 für neues Layout für "New Heltec Wifi Kit 32" (V2) mit Schalter auf Pin 23/19/22

#define USE_ORIGINAL_SERVO_VARS
  aktivieren, falls ein Automat mit Software 0.1.4 aktualisiert wird und der Servo nicht ganz schliesst.
  Der neue Code verwendet default-Werte für die Ansteuerung des Servos, um mehr Servos anzusprechen. Der
  0.1.4 Code hatte spezielle Werte, bei denen die 0-Stellung niedriger war.
  Über dieses Define können die Werte des 0.1.4 Codes aktiviert werden. Alternative: Hardware anpassen

#define ROTARY_SCALE 2   
  Verschiedene Rotary Encoder liefern unterschiedliche Increments pro Stufe. Das kann hier angepasst werden.
  Beispiele: KY-040 = 2, HW-040 = 1
  Falls kein Rotary Encoder genutzt wird, sollte das define auf 1 gesetzt werden

#define USE_ROTARY
  aktivieren, wenn ein Rotary Encoder für die Steuerung des Interface genutzt wird.

#define USE_ROTARY_SW    // Taster des Rotary benutzen
  aktivieren, wenn der Rotary auch eine Tastfunktion hat.
  ACHTUNG: falls ein Poti (siehe unten) genutzt wird, sollte diese Variable deaktiviert werden!
           Ausnahme: es wird ein dritter Taster eingebaut. Nicht unterstützt!

#define USE_POTI         // Poti benutzen -> ACHTUNG, im Normalfall auch USE_ROTARY_SW deaktivieren!
  aktivieren, wenn ein Poti statt eines Rotary für die Steuerung des Interface genutzt wird.
```
Die weiteren defines und Variablen müssen bei einer Standard-Schaltung nicht angepasst werden.


## Hardware-Aufbau

Es wird empfohlen, den Servo erst nach dem ersten Einschalten der Elekronik mit dem Quetschhahn zu verbinden!
Der Servo fährt automatisch in die Nullstellung. Diese ist im Programm nicht einstellbar und muss daher
über die Hardware (Stellung des Servoarms und Länge der Gelenkstange) eingestellt werden.
Dann kann im Modus "Handbetrieb" die maximale Öffnung ermittelt und im Setup hinterlegt werden.


## Betrieb

Grundsätzlich: Als Drehregler wird entweder ein Poti oder ein Rotary Encoder eingesetzt.
Bei Benutzung eines Rotary Encoders mit Taster wirkt der Taster aus Auswahl-Taste.
Wird ein Poti oder Rotary Encoder ohne Taster verwendet, wird der grüne Button im Setup als Auswahl
genutzt. Die Funktion "Tara" im Handbetrieb und die direkte Verstellung des Korrekturwerts und
der Füllmenge im Automatikmodus stehen nur bei einem Rotary Encoder mit integriertem Taster zur Verfügung!


Setup:
------
In Schalterstellung II (oder I je nachdem wie der 3-fach-Schalter verbaut wurde) könnne verschiedene
Grundeinstellungen vorgenommen werden:

1. Tara  
Für jede Füllmenge bzw. das entsprechende Glas kann ein Leergewicht ("Tara") definiert werden.
Die hinterlegten Tara-Werte werden im Menu angezeigt.
Einstellung: Füllmenge wählen, leeres Glas aufstellen und über die Auswahl-Taste speichern

2. Kalibieren  
Menügeführte Kalibrierung der Wägezelle

3. Korrektur  
Einstellen des Korrekturwerts. Je nach Temperatur und Konsistenz des Honigs bzw. Füllmenge im
Abfüllbehälter wird durch die Trägheit des Systems der eingestellte Wert überschritten.
Der Korrekturwert dient dazu, das anzupassen bzw. ein paar Gramm mehr einzufüllen, um die Richtlinien
zu erfüllen.  
Wert über Drehregler einstellen und mit Auswahl bestätigen

4. Füllmenge  
Auswahl der Abfüllmenge

5. Autostart  
Automatischer oder manueller Start der Abfüllung im Automatikmodus

6. Servo Max  
Der maximale Öffnungswinkel des Servos  
Hierüber wird der maximale Hub des Servos begrenzt, um den Servo an die Mechanik anzupassen.
Einstellung: Im Handbetrieb den Servo langsam auf die gewünschte maximale Öffnung fahren. Den Wert
unter "W=" dann hier einstellen.

7. Servo Fein  
Der minimale Öffnungswinkel bei der Abfüllung  
Der Servo bleibt bis zum Erreichen des Zielgewichts (Füllmenge + Korrektur) mindestens so weit geöffnet
und schliesst danach vollständig. Sollte je nach Konsistenz angepasst werden.
Ein größerer Wert hier führt zu einer schnelleren Füllung, ein kleinerer Wert zu genaueren Mengen.

8. Clear Pref's  
Setzt alle Voreinstellungen (nach Bestätigung) zurück.
Danach muss die Kalibrierung der Waage wiederholt und alle Werte neu gesetzt werden!

Handbetrieb:
------------
Im Handbetrieb wird der Öffnungswinkel des Servo direkt über den Drehregler bestimmt. Der absolute und
relative Servo-Winkel werden in der oberen Zeile angezeigt.

Der Servo wird über die grüne Taste aktiviert und über rote Taste deaktivert. Der aktuell Zustand wird
über das Play/Pause Symbol links angezeigt.

Das Gewicht wird permanent angezeigt. Über den Auswahl-Taster kann das aktuelle Gewicht als Tara eingestellt
werden, d.h. die Waage springt auf Null und zählt ab dem aktuellen Gewicht. Zurückgestellt wird über einen
Tastendruck bei leerer Waage.

Der Modus dient zum manuellen Füllen und zum ermitteln der Werte für den maximalen und minimalen Winkel
des Servos für das Setup.

Automatik:
----------
Der Automatik-Modus hat zwei Betriebsarten, die über das Setup "Autostart" gewählt werden. Der aktive
Autostart wird in der obersten Zeile mit "AS" angezeigt. Ausserdem sind dort der absolute und relative
Öffnungswinkel zu finden.

Die untere Zeile zeigt die Werte für die Korrektur und die Füllmenge.

Der Automatik-Modus muss über die grüne Taste aktiviert werden. Rot stoppt die Automatik und schliesst
den Servo.
Über den Drehregler kann der Öffnungswinkel des Servos begrenzt werden.

Autostart nicht aktiv:
----------------------
Wenn der Autostart nicht aktiv ist, dann startet der Servo nur, wenn ein leeres oder teilweise gefülltes
Glas auf der Waage steht. Dieses Glas wird bis zur eingestellten Füllmenge befüllt, dann stoppt das
Programm.

Eine erneute Befüllung muss wieder mit der grünen Taste aktiviert werden (Halbautomatik).

**Achtung:** Da auch teilgefüllte Gläser befüllt werden, kann ein abweichendes Gewicht des leeren Glases nicht
berücksichtigt werden! Ein halb volles 125g Glas kann die Befüllung mit 500g auslösen...

Autostart aktiv:
----------------
Der Autostart-Modus wird ebenfalls über die grüne Taste aktiviert (Play/Pause Symbol).  

Danach startet der Füll-Vorgang automatisch, sobald ein passendes, leeres Glas aufgesetzt wird. Wenn das
gefüllte Glas entnommen wird und ein weiteres Glas aufgesetzt wird, startet der nächste Füll-Vorgang ohne
weitere Bestätigung (Vollautomatik).

Das Gewicht das Glases wird als Tara für diesen Füllvorgang genutzt, um Schwankungen der leeren Gläser
auszugleichen.

Ein teilgefülltes Glas wird nicht befüllt!

Wenn kein Füllvorgang aktiv ist (der Hahn also geschlossen ist), kann in beiden Modi über den Auswahl-Taster
direkt der Korrekturwert und die Füllmenge zur Verstellung angewählt werden. Der jeweils zu verstellende
Wert blinkt.


## Verwendung mit PlatformIO

Just type:
```
make
```

After successfully building it, you will find firmware images at

- .pio/build/heltec/firmware.bin
- .pio/build/heltec/firmware.elf

# Halterung aus 3D Drucker
Um dies einfacher zu montieren, wurde ein 3D Drucker Model gezeichnet. Hierbei kann sehr einfach dies sehr einfach montiert werden.
https://www.thingiverse.com/thing:4564453


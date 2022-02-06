![image](https://github.com/ClemensGruber/hani-mandl/workflows/PlatformIO%20CI/badge.svg)
![image](https://img.shields.io/github/v/tag/ClemensGruber/hani-mandl.svg)

# HaniMandl mit geeichter rs232/Waage

Ein halbautomatischer Honig-Abfüll-Roboter. Diese Version nutzt geeichte Waagen, die über eine rs232-Schnittstelle angebunden sind, um den Abfüllprozess zu steuern. 

NICHT FÜR DEN PRODUKTIVEN EINSATZ. ALPHA STATUS. 

Websites:
- Code, Infos zur Hardware, Fotos und Videos initial publiziert in der Facebook-Gruppe ["Imkerei und Technik. Eigenbau"](https://www.facebook.com/groups/139671009967454)
- Weitere Infos unter: [HaniMandl, halbautomatischer Honig-Abfüllbehälter](https://community.hiveeyes.org/t/side-project-hanimandl-halbautomatischer-honig-abfullbehalter/768)

---

# Anleitung

## Einstellungen im Arduino-Code

Das Verhalten des Codes wird über mehrere `#define`-Variablen gesteuert.

```
#define WEIGHT_TYPE 1           
  0 = für den Betrieb mit HX711 + Zelle (Urform des HaniMandl)
  Mittels rs232 angeschlossene (geeichte) Waagen:
  1 = Delta-Cyprus (zB. TEM Waagen)
  X = more to come.. Geplant sind die weiteren Protokolle: TEMnative,Tisa,Dialog06,Elicom
  
  Waagen mit rs232 Schnittstelle müssen so konfiguriert werden, dass sie das
  gemessene Gewicht dauerhaft im "Delta-Cyprus"-Format übertragen.
  Die rs232-Kommunikation wird über einen max3232 hergestellt, der anstatt des HX711 
  angeschlossen wird. Die 4 ESP32-Leitungen zum HX711-Sockel (3.3V,GND, SCK, DT) können 
  direkt für den max3232 Chip (3.3V,GND,RX,TX) genutzt werden.
                              
#define HARDWARE_LEVEL 2
  1 für originales pinout Layout mit Schalter auf Pin 19/22/21
  2 für neues Layout für "New Heltec Wifi Kit 32" (V2) mit Schalter auf Pin 23/19/22
  3 für einen ESP32 WROOM mit externem 0.96", 1.3" oder 2.4" OLED (currently not supported)

#define SERVO_ERWEITERT
  aktivieren, falls ein Automat mit Software 0.1.4 aktualisiert wird und der Servo nicht ganz schliesst
  oder der Servo generell nicht weit genug bewegt wird.
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
  aktivieren, wenn ein Poti statt eines Rotary für die Steuerung des Interface genutzt wird
  Es wird dringend empfohlen, einen Rotary Encoder zu benutzen!

#define FEHLERKORREKTUR_WAAGE   // falls Gewichtssprünge auftreten, können diese hier abgefangen werden
  Achtung, kann den Wägeprozess verlangsamen. Vorher Wägezellen/HX711 prüfen!

#define QUETSCHHAHN_LINKS
  Servo invertieren, falls der Quetschhahn von links geöffnet wird. Mindestens ein Exemplar eines solchen Eimers ist bekannt
```
Die weiteren defines und Variablen müssen bei einer Standard-Schaltung nicht angepasst werden.


## Hardware-Aufbau

Es wird empfohlen, den Servo erst nach dem ersten Einschalten der Elektronik mit dem Quetschhahn zu verbinden!
Der Servo fährt automatisch in die Nullstellung. Danach kann das Gestänge verbunden werden und über das Servo-Setup
können die Servo-Positionen fein eingestellt werden.
Die rs232-Konfiguration der TEM TEKO+LCD03T-P1-B1 ist [hier](https://github.com/ClemensGruber/hani-mandl/blob/rs232/TEM-configuration/temconfig.md) beschrieben. 

## Betrieb

Grundsätzlich: Als Drehregler wird entweder ein Poti oder ein Rotary Encoder eingesetzt.
Bei Benutzung eines Rotary Encoders mit Taster wirkt der Taster aus Auswahl-Taste.
Wird ein Poti oder Rotary Encoder ohne Taster verwendet, wird der grüne Button im Setup als Auswahl
genutzt. Die Funktion "Tara" im Handbetrieb und die direkte Verstellung des Korrekturwerts und
der Füllmenge im Automatikmodus stehen nur bei einem Rotary Encoder mit integriertem Taster zur Verfügung!


Setup
-----
In Schalterstellung II (oder I je nachdem wie der 3-fach-Schalter verbaut wurde) könnnen verschiedene
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

5. Automatik
Einstellen der beiden Automatiken "Autostart" und "Autokorrektur" sowie des Kulanzwerts für die Autokorrektur.
- Autostart beginnt den Füllvorgang, wenn ein passendes, leeres Glas aufgesetzt wird
- Autokorrektur ermittelt einen automatischen Korrekturwert, um die Gläser bei sinkendem Druck im Behälter
  bis zur Füllmenge + Kulanz zu befüllen

6. Servowinkel
Definition der minimalen und maximalen Öffnungswinkel des Servos sowie des Winkels für die Feindosierung.
Der minimale Öffnungswinkel sollte den Quetschhahn mit minimalem Spiel schliessen.
Der maximale Öffnungswinkel begrenzt den Hub des Servos, um den Servo an die Mechanik anzupassen.
Der Servo bleibt bis zum Erreichen des Zielgewichts (Füllmenge + Korrektur) mindestens bis zum Winkel Feindosierung
geöffnet und schliesst danach vollständig. Sollte je nach Konsistenz angepasst werden. Ein größerer Wert hier
führt zu einer schnelleren Füllung, ein kleinerer Wert zu genaueren Mengen.

Einstellung: Im Handbetrieb den Servo langsam auf die gewünschte Öffnung fahren. Den Wert unter "W=" dann hier
einstellen.
Über den Punkt "Livesetup" können die Winkel auch direkt aus dem Setup angefahren werden. Vorsicht bei Verwendung
mit einem Poti, oder bei gefülltem Abfüllbehälter!

7. Clear Pref's
Setzt alle Voreinstellungen (nach Bestätigung) zurück.
Danach muss die Kalibrierung der Waage wiederholt und alle Werte neu gesetzt werden!

Handbetrieb
-----------
Im Handbetrieb wird der Öffnungswinkel des Servo direkt über den Drehregler bestimmt. Der absolute und
relative Servo-Winkel werden in der oberen Zeile angezeigt.

Der Servo wird über die grüne Taste aktiviert und über rote Taste deaktivert. Der aktuelle Zustand wird
über das Play/Pause Symbol links angezeigt.

Das Gewicht wird permanent angezeigt. Über den Auswahl-Taster kann das aktuelle Gewicht als Tara eingestellt
werden, d.h. die Waage springt auf Null und zählt ab dem aktuellen Gewicht. Zurückgestellt wird über einen
Tastendruck bei leerer Waage.

Der Modus dient zum manuellen Füllen und zum ermitteln der Werte für den maximalen und minimalen Winkel
des Servos für das Setup.

Automatik
---------
Der Automatik-Modus hat zwei Betriebsarten, die über das Setup "Autostart" gewählt werden. Der aktive
Autostart wird in der obersten Zeile mit "AS" angezeigt. Ausserdem sind dort der absolute und relative
Öffnungswinkel zu finden.

Die untere Zeile zeigt die Werte für die Korrektur und die Füllmenge. Eine aktive Autokorrektur ist dort
erkennbar, weil der Korrekturwert dort statt "k=" mit "ak=" angegeben wird.

Der Automatik-Modus muss über die grüne Taste aktiviert werden. Rot stoppt die Automatik und schliesst
den Servo.
Über den Drehregler kann der Öffnungswinkel des Servos begrenzt werden.

Autostart nicht aktiv
---------------------
Wenn der Autostart nicht aktiv ist, dann startet der Servo nur, wenn ein leeres oder teilweise gefülltes
Glas auf der Waage steht. Dieses Glas wird bis zur eingestellten Füllmenge befüllt, dann stoppt das
Programm.

Eine erneute Befüllung muss wieder mit der grünen Taste aktiviert werden (Halbautomatik).

**Achtung:** Da auch teilgefüllte Gläser befüllt werden, kann ein abweichendes Gewicht des leeren Glases nicht
berücksichtigt werden! Ein halb volles 125g Glas kann die Befüllung mit 500g auslösen...

Autostart aktiv
---------------
Der Autostart-Modus wird ebenfalls über die grüne Taste aktiviert (Play/Pause Symbol).

Danach startet der Füll-Vorgang automatisch, sobald ein passendes, leeres Glas aufgesetzt wird. Wenn das
gefüllte Glas entnommen wird und ein weiteres Glas aufgesetzt wird, startet der nächste Füll-Vorgang ohne
weitere Bestätigung (Vollautomatik).

Das Gewicht das Glases wird als Tara für diesen Füllvorgang genutzt, um Schwankungen der leeren Gläser
auszugleichen.

Ein teilgefülltes Glas wird nicht befüllt! Durch erneutes Drücken der Start-Taste wird die Befüllung erzwungen.

Wenn kein Füllvorgang aktiv ist (der Hahn also geschlossen ist), kann in beiden Modi über den Auswahl-Taster
direkt der Korrekturwert und die Füllmenge zur Verstellung angewählt werden. Der jeweils zu verstellende
Wert blinkt.

Autokorrektur
-------------
Die aktive Autokorrektur (Anzeige "ak=" unten links) führt die Füllmenge automatisch mit dem sinkenden Druck
im Abfüllbehälter nach. Das Zielgewicht wird im Automatik-Setup über die Kulanz eingestellt (Füllmenge + Kulanz).


## Notizen
- Das Projekt wurde auf dem [Heltec WiFi Kit 32](https://community.hiveeyes.org/t/heltec-wifi-kit-32-esp32-mit-kleinem-oled/1498), einem ESP32 mit OLED-Display entwickelt, sollte jedoch auch auf anderen ESP32 Geräten lauffähig sein.
- Bitte nutzt die neueste Version der HX711 Bibliothek von https://github.com/bogde/HX711, die ältere war nicht ohne weiteres auf einem ESP32 lauffähig.


## Firmware bauen

Just type:
```
make
```

After successfully building it, you will find firmware images at

- .pio/build/heltec/firmware.bin
- .pio/build/heltec/firmware.elf


## Binär-Datei `hani-mandl.bin`

Die Datei `hani-mandl.bin` wurde mit folgenden Parametern für das Board Heltec ESP32 Arduino > Wifi Kit 32 compiliert:

```
#define HARDWARE_LEVEL 2        // 1 = originales Layout mit Schalter auf Pin 19/22/21
                                // 2 = Layout für V2 mit Schalter auf Pin 23/19/22
#define SERVO_ERWEITERT         // definieren, falls die Hardware mit dem alten Programmcode mit Poti aufgebaut wurde oder der Servo zu wenig fährt
                                // Sonst bleibt der Servo in Stop-Position einige Grad offen! Nach dem Update erst prüfen!
#define ROTARY_SCALE 2          // in welchen Schritten springt unser Rotary Encoder.
                                // Beispiele: KY-040 = 2, HW-040 = 1, für Poti-Betrieb auf 1 setzen
#define USE_ROTARY              // Rotary benutzen
#define USE_ROTARY_SW           // Taster des Rotary benutzen
```

Eine Anleitung zum Flashen der Binär-Datei gibt es unter
http://hanimandl.de/2020/12/23/firmware-binary-flashen/

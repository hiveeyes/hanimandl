#define WEIGHT_TYPE 2           // 0 = HX711 + Zelle
                                // max3232 + geeichte Waage über rs232 angeschlossen:
                                // 1 = Delta-Cyprus
                                // 2 = TEM
                                // 3 = Tisa (currently NOT supported)
                                // 4 = Dialog06 (currently NOT supported)
                                // 5 = Elicom (currently NOT supported)
//
// Hier den Code auf die verwendete Hardware einstellen
//

#define HARDWARE_LEVEL 1        // 1 = originales Layout mit Schalter auf Pin 19/22/21
                                // 2 = Layout für V2 mit Schalter auf Pin 23/19/22
                                // 3 = ESP32 WROOM-32 mit externem 0.96", 1.3" oder 2.4" OLED
#define DISPLAY 2               // 1 = Heltec mit 0.96" oder 1.3" OLED per I2C
                                // 2 = ESP32-WROOM mit 0.96" oder 1.3" OLED per I2C
                                // 3 = 2.24" OLED per I2C
                                // 4 = 2.24" OLED per SPI
#define SERVO_ERWEITERT         // definieren, falls die Hardware mit dem alten Programmcode mit Poti aufgebaut wurde oder der Servo zu wenig fährt
                                // Sonst bleibt der Servo in Stop-Position einige Grad offen! Nach dem Update erst prüfen!
#define ROTARY_SCALE 1          // in welchen Schritten springt unser Rotary Encoder. 
                                // Beispiele: KY-040 = 2, HW-040 = 1, für Poti-Betrieb auf 1 setzen
#define USE_ROTARY              // Rotary benutzen
#define USE_ROTARY_SW           // Taster des Rotary benutzen
#define ROTARY_AS_THREAD        // Drehungen des Rotary in einem Thread auf Kern0 und nicht über Interrupts verarbeiten 
//#define USE_POTI              // Poti benutzen -> ACHTUNG, im Normalfall auch USE_ROTARY_SW deaktivieren!
//#define FEHLERKORREKTUR_WAAGE   // falls Gewichtssprünge auftreten, können diese hier abgefangen werden
                                // Achtung, kann den Wägeprozess verlangsamen. Vorher Hardware prüfen.
//#define QUETSCHHAHN_LINKS       // Servo invertieren, falls der Quetschhahn von links geöffnet wird. Mindestens ein Exemplar bekannt
//
// Ende Benutzereinstellungen!
// 

//
// Ab hier nur verstellen wenn Du genau weisst, was Du tust!
//
//#define isDebug 3             // serielle debug-Ausgabe aktivieren. Mit >3 wird jeder Messdurchlauf ausgegeben
                                // ACHTUNG: zu viel Serieller Output kann einen ISR-Watchdog Reset auslösen!
//#define POTISCALE             // Poti simuliert eine Wägezelle, nur für Testbetrieb!
#define MAXIMALGEWICHT 1000     // Maximales Gewicht

// Ansteuerung der Waage
#define SCALE_READS 2      // Parameter für hx711 Library. Messwert wird aus der Anzahl gemittelt
//#define SCALE_GETUNITS(n)  (waage_vorhanden ? round(scale.get_units(n)) : simulate_scale(n) )
//#define SCALE_GETUNITS(n)  (waage_vorhanden ? getWeight(n) : simulate_scale(n) )
#define SCALE_GETUNITS(n)  getWeight(n)


// Ansteuerung Servo
#ifdef QUETSCHHAHN_LINKS
#define SERVO_WRITE(n)     servo.write(180-n)
#else
#define SERVO_WRITE(n)     servo.write(n)
#endif

// Rotary Encoder Taster zieht Pegel auf Low, Start/Stop auf High!
#ifdef USE_ROTARY_SW
#define SELECT_SW outputSW
#define SELECT_PEGEL LOW
#else
#define SELECT_SW button_start_pin
#define SELECT_PEGEL HIGH
#endif

// Betriebsmodi 
#define MODE_SETUP       0
#define MODE_AUTOMATIK   1
#define MODE_HANDBETRIEB 2

// Buzzer Sounds
#define BUZZER_SHORT   1
#define BUZZER_LONG    2
#define BUZZER_SUCCESS 3
#define BUZZER_ERROR   4


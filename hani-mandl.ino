/*
  Abfuellwaage Version 0.2.2
  --------------------------
  Copyright (C) 2018-2019 by Marc Vasterling, Marc Wetzel, Clemens Gruber  
            
  2018-05 Marc Vasterling | initial version, 
                            published in the Facebook group "Imkerei und Technik. Eigenbau",
                            Marc Vasterling: "meinen Code kann jeder frei verwenden, ändern und hochladen wo er will, solange er nicht seinen eigenen Namen drüber setzt."
  2018-06 Marc Vasterling | improved version, 
                            published in the Facebook group also
  2019-01 Marc Wetzel     | Refakturierung und Dokumentation, 
                            published in the Facebook group also
  2019-02 Clemens Gruber  | code beautifying mit kleineren Umbenennungen bei Funktionen und Variablen
                            Anpssung fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
                            - pins bei OLED-Initialisierung geaendert
                            - pins geaendert, um Konflikte mit hard wired pins des OLEDs zu vermeiden 
  2019-02 Clemens Gruber  | Aktivierung der internen pull downs für alle digitalen Eingaenge
  2019-02 Clemens Gruber  | "normale" pins zu Vcc / GND geaendert um die Verkabelung etwas einfacher und angenehmer zu machen
  2020-05 Andreas Holzhammer | Anpassungen an das veränderte ;-( pin-Layout der Version 2 des Heltec 
                               wird verkauft als "New Wifi Kit 32" oder "Wifi Kit 32 V2"
                               - Änderungen siehe https://community.hiveeyes.org/t/side-project-hanimandl-halbautomatischer-honig-abfullbehalter/768/43 
                                 und https://community.hiveeyes.org/t/side-project-hanimandl-halbautomatischer-honig-abfullbehalter/768/44
                               - der code ist mit der geänderten pin-Belegung nicht mehr abwärskompatibel zur alten Heltec-Version   
  2020-05 Andreas Holzhammer | Tara pro abzufüllendem Glas automatisch anpassen (Variable tara_glas)
                               Code läuft auch ohne Waage
  2020-06 Andreas Holzhammer | Grosser Codeumbau, Anpassung auf Rotary Encoder
                               - Tara pro Glas einstellbar
                               - Öffnungswinkel für Maximale Öffnung und Feindosierung im Setup konfigurierbar
                               - Korrektur und Glasgröße im Automatikmodus per Rotary Encoder Button wählbar
                               - Preferences löschbar über Setup
 
  This code is in the public domain.
   
  Hinweise zur Hardware
  ---------------------
  - bei allen digitalen Eingänge sind interne pull downs aktiviert, keine externen-Widerständen nötig! 
*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>      /* aus dem Bibliotheksverwalter */
#include <HX711.h>        /* https://github.com/bogde/HX711 */
#include <ESP32_Servo.h>  /* https://github.com/jkb-git/ESP32Servo */
#include <Preferences.h>  /* aus dem BSP von expressif */

#define isDebug 
#undef Autokorrektur
#undef HARDWARE_V1       // originales Layout, Schalter auf Pin 19/22/21

#define MODE_SETUP       0
#define MODE_AUTOMATIK   1
#define MODE_HANDBETRIEB 2

#define SCALE_READS 3     // Parameter für scale.read_average()
#define SCALE_READAVERAGE(n) (waage_vorhanden ? scale.read_average(n) : simulate_scale(n) )

Servo servo;
HX711 scale;
Preferences preferences;

// ** Definition der pins 
// ----------------------

// OLED
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
// fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Rotary
#define outputA 33
#define outputB 26
#define outputSW 32
volatile int counter_pos = 50; 
int aState;
int aLastState;  
static boolean rotating = false;      // debounce management

struct rotary {
  int Value;
  int Minimum;
  int Maximum;
  int Step;
};

#define SW_WINKEL    0
#define SW_KORREKTUR 1
#define SW_MENU      2
struct rotary rotaries[3] = { { 0,   0, 100, 5 },     // Winkel
                              { 0, -20,  20, 1 },     // Korrektur
                              { 0,   0,   5, 1 } };   // für Menuauswahlen
int rotary_select = SW_WINKEL;

struct glas { 
  int Gewicht;
  int Tara;
};

struct glas glaeser[5] = { {  125, -9999 },
                           {  250, -9999 },
                           {  375, -9999 },
                           {  500, -9999 },
                           { 1000, -9999 } };


// Servo
const int servo_pin = 2;

#ifdef HARDWARE_V1
// 3x Schalter Ein 1 - Aus - Ein 2
const int switch_betrieb_pin = 19;
const int switch_vcc_pin = 22;        // <- Vcc 
const int switch_setup_pin = 21;
#else
// 3x Schalter Ein 1 - Aus - Ein 2
const int switch_betrieb_pin = 23;
const int switch_vcc_pin = 19;        // <- Vcc 
const int switch_setup_pin = 22;
// Vext control pin
const int vext_ctrl_pin = 21;
#endif

// Taster 
const int button_start_vcc_pin = 13;  // <- Vcc 
const int button_start_pin = 12;
const int button_stop_vcc_pin = 14;   // <- Vcc 
const int button_stop_pin = 27;

// Poti
const int poti_pin = 39;

// Wägezelle-IC 
const int hx711_sck_pin = 17;
const int hx711_dt_pin = 5;

int i;
int pos;
int gewicht;
int tara;             // Tara für das Standard-Glas, für Automatikmodus
int tara_glas;        // Tara für das aktuelle Glas, falls Glasgewicht abweicht
int tara_raw;         // Tara der leeren Waage
int gewicht_raw;      // Gewicht Raw-Wert, nur zur Kalkulation tara_raw
float faktor;         // Skalierungsfaktor für Werte der Waage
int faktor2;          // faktor * 10000 für Preferences
int fmenge;
int fmenge_index;
int korrektur;
int autostart;
int winkel;                    // aktueller Servo-Winkel
int winkel_hard_min = 0;       // Hard-Limit für Servo
int winkel_hard_max = 155;     // Hard-Limit für Servo
int winkel_min = 0;            // nicht einstellbar, wird per Hardware angepasst!
int winkel_max = 85;           // konfigurierbar im Setup
int winkel_dosier_min = 35;    // konfigurierbar im Setup
float fein_dosier_gewicht = 60; // float wegen Berechnung des Schliesswinkels
int servo_aktiv = 0;            // Servo aktivieren ja/nein
char ausgabe[30]; // Fontsize 12 = 13 Zeichen maximal in einer Zeile
int modus = -1;   // Bei Modus-Wechsel den Servo auf Minimum fahren
int autofill = 0; // Für Automatikmodus - System ein/aus?
int waage_vorhanden = 0;

// Simuliert die Dauer des Wägeprozess, wenn keine Waage angeschlossen ist. Wirkt sich auf die Blinkfrequenz im Automatikmodus aus.
int simulate_scale(int n) {
//    while (n >= 1) { 
//      delay(80);
//      n--;
//    }
    return 9500;
}

// Rotary Taster. Der Interrupt kommt nur im Automatikmodus zum Tragen und nur wenn der Servo inaktiv ist. 
void IRAM_ATTR isr1() {
  static unsigned long last_interrupt_time = 0; 
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 300) {   // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if ( modus == MODE_AUTOMATIK && servo_aktiv == 0 ) {          // nur im Automatik-Modus interessiert uns der Click
      rotary_select = (rotary_select + 1) % 3;
#ifdef isDebug
    Serial.print("Rotary Button changed to ");
    Serial.println(rotary_select);
#endif 
    }
    last_interrupt_time = interrupt_time;
  }
}

// Rotary Encoder. Der Taster schaltet in einen von drei Modi, in denen unterschiedliche Werte gezählt werden.
// SW_WINKEL    = Einstellung des Servo-Winkels
// SW_KORREKTUR = Korrekturfaktor für Füllgewicht
// SW_MENU      = Zähler für Menuauswahlen 
void IRAM_ATTR isr2() { 
  if ( rotating ) delay (10);  // wait a little until the bouncing is done
   
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
    if (aState != aLastState) {     
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      if (digitalRead(outputB) != aState) {
         rotaries[rotary_select].Value -= rotaries[rotary_select].Step;
      } else {    // counter-clockwise
         rotaries[rotary_select].Value += rotaries[rotary_select].Step;
      }
      if ( rotaries[rotary_select].Value < rotaries[rotary_select].Minimum ) { rotaries[rotary_select].Value = rotaries[rotary_select].Minimum; }
      if ( rotaries[rotary_select].Value > rotaries[rotary_select].Maximum ) { rotaries[rotary_select].Value = rotaries[rotary_select].Maximum; }

      rotating = false;
#ifdef isDebug
      Serial.print("Rotary Value changed to ");
      Serial.println(rotaries[rotary_select].Value);
#endif 
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
}

void getPreferences(void) {
    preferences.begin("EEPROM", false);          // Parameter aus dem EEPROM lesen
    faktor2 = preferences.getUInt("faktor2", 0); // falls das nicht gesetzt ist -> Waage ist nicht kalibriert

#ifdef isDebug
    if (faktor2 == 0) {
      Serial.println("Waage ist nicht kalibiert!");
//        for (int i=0; i < 200; i++) {
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
//        }
      faktor2 = 10000;   // Für brauchbare Anzeige, sonst Division durch Null!
    }
#endif
  
    faktor = (faktor2 / 10000.00);
    pos       = preferences.getUInt("pos", 0);
    tara      = preferences.getUInt("tara", 0);
    tara_raw  = preferences.getUInt("tara_raw", 0);
    fmenge    = preferences.getUInt("fmenge", 0);
    korrektur = preferences.getUInt("korrektur", 0);
    autostart = preferences.getUInt("autostart", 0);
    fmenge_index      = preferences.getUInt("fmenge_index", 2);
    winkel_max        = preferences.getUInt("winkel_max", winkel_max);
    winkel_dosier_min = preferences.getUInt("winkel_fein", winkel_dosier_min);

    i = 0;
    while( i < 5 ) {
      sprintf(ausgabe, "tara%d", i);
      glaeser[i].Tara= preferences.getInt(ausgabe, -9999);
      i++;
    }

    preferences.end();

    // Parameter für den Rotary Encoder setzen
    rotaries[SW_KORREKTUR].Value = korrektur;
    rotaries[SW_WINKEL].Value = pos;
    rotaries[SW_MENU].Value = fmenge_index;
    
#ifdef isDebug
    Serial.println("Preferences:");
    Serial.print("pos = ");          Serial.println(pos);
    Serial.print("faktor = ");       Serial.println(faktor);
    Serial.print("tara = ");         Serial.println(tara);
    Serial.print("tara_raw = ");     Serial.println(tara_raw);
    Serial.print("fmenge = ");       Serial.println(fmenge);
    Serial.print("korrektur = ");    Serial.println(korrektur);
    Serial.print("autostart = ");    Serial.println(autostart);
    Serial.print("fmenge_index = "); Serial.println(fmenge_index);
    Serial.print("winkel_max = ");   Serial.println(winkel_max);
    Serial.print("winkel_fein = ");  Serial.println(winkel_dosier_min);

    i = 0;
    while( i < 5 ) {
      sprintf(ausgabe, "tara%d = ", i);
      Serial.print(ausgabe);         Serial.println(glaeser[i].Tara);
      i++;
    }
#endif
}

void setPreferences(void) {
    preferences.begin("EEPROM", false);

    preferences.putUInt("pos", pos);
    preferences.putUInt("tara", tara);
    preferences.putUInt("tara_raw", tara_raw);
    preferences.putUInt("fmenge", fmenge);
    preferences.putUInt("korrektur", korrektur);
    preferences.putUInt("autostart", autostart);
    preferences.putUInt("faktor2", faktor2);
    preferences.putUInt("winkel_max", winkel_max);
    preferences.putUInt("winkel_fein", winkel_dosier_min);
    preferences.putUInt("fmenge_index", fmenge_index);

    i = 0;
    while( i < 5 ) {
      sprintf(ausgabe, "tara%d", i);
      preferences.putInt(ausgabe, glaeser[i].Tara);
      i++;
    }

    preferences.end();
}

void clearPreferences(void) {
    preferences.begin("EEPROM", false);
    preferences.clear();
    preferences.end();
}   

void setupTara(void) {
    int j;
    rotaries[SW_MENU] = { 0, 0, 4, -1};      // Set Encoder to Menu Mode, four Selections, inverted count

    i = 0;
    while ( i == 0 ) {
      u8g2.setFont(u8g2_font_courB10_tf);
      u8g2.clearBuffer();

      j = 0;
      while( j < 5  ) {
        u8g2.setCursor(10, 10+(j*13));   u8g2.print( glaeser[j].Gewicht);
        u8g2.setCursor(65, 10+(j*13));   u8g2.print((glaeser[j].Tara < 0) ? "not set" : "set");
        j++;
      }
      u8g2.setCursor(0, 10+((rotaries[SW_MENU].Value)*13));    
      u8g2.print("*");
      u8g2.sendBuffer();

      if ( digitalRead(outputSW) == LOW ) {
        glaeser[rotaries[SW_MENU].Value].Tara = ((int(SCALE_READAVERAGE(30)) - tara_raw) / faktor);

        u8g2.clearBuffer();
        j = 0;
        while( j < 5  ) {
          u8g2.setCursor(10, 10+(j*13));   u8g2.print( glaeser[j].Gewicht);
          u8g2.setCursor(65, 10+(j*13));   u8g2.print((glaeser[j].Tara < 0) ? "not set" : "set");
          j++;
        }
        u8g2.setCursor(0, 10+((rotaries[SW_MENU].Value)*13));    
        u8g2.print("*");
        u8g2.sendBuffer();
       
        delay(1000);
        i++;
      }  
    }
}

void setupCalibration(void) {
    u8g2.setFont(u8g2_font_courB12_tf);
    u8g2.clearBuffer();
    u8g2.setCursor(0, 12);    u8g2.print("Bitte 500g");
    u8g2.setCursor(0, 28);    u8g2.print("aufstellen");
    u8g2.setCursor(0, 44);    u8g2.print("und mit Start");
    u8g2.setCursor(0, 60);    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    i = 1;
    while (i > 0) {
      if ((digitalRead(outputSW)) == LOW) {
//      if ((digitalRead(button_start_pin)) == HIGH) {
        gewicht_raw = (int(SCALE_READAVERAGE(30)));
        delay(1000);
        i = 0;
      }
    }
    
    u8g2.clearBuffer();
    u8g2.setCursor(0, 12);    u8g2.print("Bitte Waage");
    u8g2.setCursor(0, 28);    u8g2.print("leeren");
    u8g2.setCursor(0, 44);    u8g2.print("und mit Start");
    u8g2.setCursor(0, 60);    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    i = 1;
    while (i > 0) {
      if ((digitalRead(outputSW)) == LOW) {
//      if ((digitalRead(button_start_pin)) == HIGH) {
        tara_raw = (int(SCALE_READAVERAGE(30)));
        delay(1000);
        i = 0;
        faktor = ((gewicht_raw - tara_raw) / 500.000);

        preferences.begin("EEPROM", false);  // faktor und tara ins eeprom schreiben
        preferences.putUInt("faktor2", (faktor * 10000));
        preferences.putUInt("tara_raw", tara_raw);
        preferences.end();
      }
    }
}

void setupKorrektur(void) {
    rotary_select = SW_KORREKTUR;

    i = 1;
    while (i > 0) {
//      pos = (map(analogRead(poti_pin), 0, 4095, 10, -50));
      pos = rotaries[SW_KORREKTUR].Value;
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print("Korrektur");
      u8g2.setCursor(40, 28);
      u8g2.print(pos);
      u8g2.sendBuffer();
      
//      if ((digitalRead(button_start_pin)) == HIGH) {
      if ((digitalRead(outputSW)) == LOW) {
        korrektur = pos;
        u8g2.setCursor(100, 28);
        u8g2.print("OK");
        u8g2.sendBuffer();
        delay(2000);
        i = 0;
      }
      
      preferences.begin("EEPROM", false);
      preferences.putUInt("korrektur", korrektur);
      preferences.end();
    }
    rotary_select = SW_MENU;
}

// Funktion zum anpassen eines beliebigen Zahlwerts (Öffnungswinkel Maximum und Feindosierung) 
// Könnte auch für Korrektur genutzt werden, der Wert hat aber seine eigene Datenstruktur
void setupZahlwert(int *param, int min, int max, char *name) {
    rotaries[SW_MENU] = { *param, min, max, 1};      // Set Encoder to Menu Mode, two Selections, inverted count
        
    i = 1;
    while (i > 0) {
//      pos = (map(analogRead(poti_pin), 0, 4095, 10, -50));
      pos = rotaries[SW_MENU].Value;
      u8g2.setFont(u8g2_font_courB12_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print(name);
      u8g2.setCursor(40, 28);
      u8g2.print(pos);
      u8g2.sendBuffer();
      
//      if ((digitalRead(button_start_pin)) == HIGH) {
      if ((digitalRead(outputSW)) == LOW) {
        *param = pos;
        u8g2.setCursor(100, 28);
        u8g2.print("OK");
        u8g2.sendBuffer();
        delay(1000);
        i = 0;
      }
    }
}


void setupFuellmenge(void) {
    int j;
    rotaries[SW_MENU] = { 0, 0, 4, -1};      // Set Encoder to Menu Mode, four Selections, inverted count
    
    u8g2.setFont(u8g2_font_courB10_tf);
    i = 1;
    while (i > 0) {
//      pos = (map(analogRead(poti_pin), 0, 4095, 1, 4));
      pos = rotaries[SW_MENU].Value;

      u8g2.clearBuffer();
      j = 0;
      while( j < 5  ) {
        u8g2.setCursor(10, 10+(j*13));    
        u8g2.print(glaeser[j].Gewicht);
        j++;
      }
      u8g2.setCursor(0, 10+((rotaries[SW_MENU].Value)*13));    
      u8g2.print("*");
      u8g2.sendBuffer();

      if ( digitalRead(outputSW) == LOW ) {
        fmenge = glaeser[pos].Gewicht;
        tara   = glaeser[pos].Tara;
        fmenge_index = pos; 
        
        u8g2.setCursor(100, 10+(rotaries[SW_MENU].Value*13));
        u8g2.print("OK");
        u8g2.sendBuffer();
        delay(1000);
        i = 0;
      }
    }
}

void setupAutostart(void) {
  rotaries[SW_MENU] = { 1, 1, 2, -1};      // Set Encoder to Menu Mode, two Selections, inverted count

  i = 1;
  while (i > 0) {
//    pos = (map(analogRead(poti_pin), 0, 4095, 1, 2));
    pos = rotaries[SW_MENU].Value;
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);    u8g2.print("Auto EIN");
    u8g2.setCursor(10, 28);    u8g2.print("Auto AUS");
    
    u8g2.setCursor(0, 12+((pos-1)*16));
    u8g2.print("*");
    u8g2.sendBuffer();
 
    if ((digitalRead(outputSW)) == LOW) {
//      if ((digitalRead(button_start_pin)) == HIGH) {
      if (pos == 1) { autostart = 1; }
      if (pos == 2) { autostart = 2; }

      u8g2.setCursor(105, 12+((pos-1)*16));
      u8g2.print("OK");
      u8g2.sendBuffer();
      delay(2000);
      i = 0;
      preferences.begin("EEPROM", false);
      preferences.putUInt("autostart", autostart);
      preferences.end();
    }
  }
}

void processSetup(void) {
  if ( modus != MODE_SETUP ) {
     modus = MODE_SETUP;
     winkel = winkel_min;          // Hahn schliessen
     servo_aktiv = 0;                          // Servo-Betrieb aus
     servo.write(winkel);
     rotaries[SW_MENU] = { 1, 1, 8, -1};
     rotary_select = SW_MENU;
  }
//  pos = (map(analogRead(poti_pin), 0, 4095, 1, 5));
  pos = rotaries[SW_MENU].Value;

  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  if( pos < 6 ) {
     u8g2.setCursor(10, 10);   u8g2.print("Tara");
     u8g2.setCursor(10, 23);   u8g2.print("Kalibrieren");
     u8g2.setCursor(10, 36);   u8g2.print("Korrektur");
     u8g2.setCursor(10, 49);   u8g2.print("Fuellmenge");
     u8g2.setCursor(10, 62);   u8g2.print("Autostart");
     u8g2.setFont(u8g2_font_open_iconic_arrow_2x_t);
     u8g2.drawGlyph(112, 64, 0x40);  
  } else {
     u8g2.setCursor(10, 10);   u8g2.print("Servo Max");
     u8g2.setCursor(10, 23);   u8g2.print("Servo Fein");
     u8g2.setCursor(10, 36);   u8g2.print("Clear Pref's");
     u8g2.setFont(u8g2_font_open_iconic_arrow_2x_t);
     u8g2.drawGlyph(112, 16, 0x43);  
  }
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(0, 10 + (((pos-1)%5) * 13));
  u8g2.print("*");
  u8g2.sendBuffer();

//  if ((digitalRead(button_start_pin)) == HIGH) {
  if ( digitalRead(outputSW) == LOW ) {
    
    // sollte verhindern, dass ein Tastendruck gleich einen Unterpunkt wählt
    while( digitalRead(outputSW) == LOW ) {
    }

    int lastpos = pos;
    if (pos == 1)   setupTara();              // Tara 
    if (pos == 2)   setupCalibration();       // Kalibrieren 
    if (pos == 3)   setupKorrektur();         // Korrektur 
    if (pos == 4)   setupFuellmenge();        // Füllmenge 
    if (pos == 5)   setupAutostart();         // Autostart 
    if (pos == 6)   setupZahlwert(&winkel_max, winkel_dosier_min, winkel_hard_max, "Servo Max" );  // Maximaler Öffnungswinkel
    if (pos == 7)   setupZahlwert(&winkel_dosier_min, winkel_hard_min, winkel_max, "Servo Fein" ); // Minimaler Abfüllwinkel
    setPreferences();

    if (pos == 8)   clearPreferences();       // EEPROM löschen
    rotaries[SW_MENU] = { lastpos, 1, 8, -1}; // Menu-Parameter könnten verstellt worden sein
  }
}

void processAutomatik(void) {
  int zielgewicht;           // Glas + Korrektur
  int time;
#ifdef Autokorrektur
  static int gewicht_vorher; // Gewicht des vorher gefüllten Glases
  static int time_vorher;
  static int zielgewicht_vorher;
#endif

  if ( modus != MODE_AUTOMATIK ) {
     modus = MODE_AUTOMATIK;
     winkel = winkel_min;          // Hahn schliessen
     servo_aktiv = 0;              // Servo-Betrieb aus
     servo.write(winkel);
     autofill = 0;                 // automatische Füllung starten
     tara_glas = 0;
     rotary_select = SW_WINKEL;                      // Einstellung für Winkel über Rotary
     rotaries[SW_MENU] = { fmenge_index, 0, 4, 1 };  // Einstellung für Füllmenge über Rotary
  }

//  pos = (map(analogRead(poti_pin), 0, 4095, 100, 0));
  pos =          rotaries[SW_WINKEL].Value;
  korrektur =    rotaries[SW_KORREKTUR].Value;
  fmenge_index = rotaries[SW_MENU].Value;
  tara   = glaeser[fmenge_index].Tara;
  fmenge = glaeser[fmenge_index].Gewicht;

  if ((digitalRead(button_start_pin)) == HIGH) {
    autofill      = 1;             // automatisches Füllen aktivieren
    rotary_select = SW_WINKEL;     // falls während der Parameter-Änderung auf Start gedrückt wird    
    setPreferences();              // falls Parameter über den Rotary verändert wurden
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    winkel      = winkel_min;
    servo_aktiv = 0;
    autofill    = 0;
    tara_glas   = 0;
  }

  gewicht = ((((int(SCALE_READAVERAGE(SCALE_READS))) - tara_raw) / faktor) - tara);
#ifdef Autokorrektur
  // für eine halbe Sekunde das Nachtropfen messen
  if ( (time_vorher - millis() < 500) ) {
    if ( abs(gewicht - gewicht_vorher) < 10 ) { 
      gewicht_vorher = gewicht;
    }
  }
#endif
  
  // Glas entfernt -> Servo schliessen
  if (gewicht < -20) {
    winkel      = winkel_min;
    servo_aktiv = 0;
    tara_glas   = 0;
    if ( autostart != 1 ) {  // Autostart nicht aktiv
      autofill  = 0;
    }
  }

  // Vollautomatik ein, leeres Glas aufgesetzt, Servo aus -> Glas füllen
  if ((autofill == 1) && (gewicht <= 5) && (gewicht >= -5) && (servo_aktiv == 0)) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB24_tf);
    u8g2.setCursor(15, 43);
    u8g2.print("START");
    u8g2.sendBuffer();
    // kurz warten und prüfen ob das Gewicht nicht nur eine zufällige Schwankung war 
    delay(1500);  
    gewicht = ((((int(SCALE_READAVERAGE(SCALE_READS))) - tara_raw) / faktor) - tara);

    if ((gewicht <= 5) && (gewicht >= -5)) {
      tara_glas   = gewicht;
#ifdef Autokorrektur
      rotaries[SW_KORREKTUR].Value -= ((gewicht_vorher - zielgewicht_vorher)/2);
#endif
      servo_aktiv = 1;
    }
  }

  zielgewicht = fmenge + korrektur + tara_glas;

  // Füll-Automatik ohne Autostart ist aktiviert, Glas ist teilweise gefüllt
  // Füllvorgang fortsetzen
  if ((autofill == 1) && (gewicht >= 0) && (autostart != 1)) {
    servo_aktiv = 1;
  }
  
  if (servo_aktiv == 1) {
    winkel = ((winkel_max * pos) / 100);
  }
  
  if ((servo_aktiv == 1) && (fmenge - (gewicht - korrektur - tara_glas) <= fein_dosier_gewicht)) {
    winkel = ( ((winkel_max * pos) / 100)
        * ( (fmenge - (gewicht - korrektur - tara_glas)) / fein_dosier_gewicht) );
  }
  
  if ((servo_aktiv == 1) && (winkel <= winkel_dosier_min)) {
    winkel = winkel_dosier_min;
  }
  
  // Glas ist voll
  if ((servo_aktiv == 1) && ((gewicht - korrektur - tara_glas) >= fmenge)) {
    winkel      = winkel_min;
    servo_aktiv = 0;
    tara_glas   = 0;
    if ( autostart != 1 ) {
      autofill = 0;
    }
#ifdef Autokorrektur
    time_vorher = millis();
    gewicht_vorher = gewicht;
    zielgewicht_vorher = zielgewicht;
#endif
  }
  
  servo.write(winkel);
  
  #ifdef isDebug
    Serial.print(" Tara_raw:");
    Serial.print(tara_raw);
    Serial.print(" Tara_glas:");
    Serial.print(tara_glas);
    Serial.print(" Faktor ");
    Serial.print(faktor);
    Serial.print(" Gewicht ");
    Serial.print(gewicht);
    Serial.print(" Zielgewicht ");
    Serial.print(zielgewicht);
    Serial.print(" Winkel ");
    Serial.println(winkel);
  #endif

  u8g2.clearBuffer();
  
  u8g2.setFont(u8g2_font_courB24_tf);
  u8g2.setCursor(10, 42);
  sprintf(ausgabe,"%5dg", gewicht - tara_glas);
  u8g2.print(ausgabe);

  u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
  u8g2.drawGlyph(0, 40, (autofill==1)?0x45:0x44 );

  u8g2.setFont(u8g2_font_courB12_tf);

  u8g2.setCursor(0, 11);
  sprintf(ausgabe,"W=%-3d %2s %3d%%", winkel, (autostart==1)?"AS":"  ", rotaries[SW_WINKEL].Value);
  u8g2.print(ausgabe);

  time = millis() % 9;
  u8g2.setCursor(0, 64);   // blinken des ausgewählten Parameters 
  if( rotary_select == SW_KORREKTUR && time < 3 ) {
    sprintf(ausgabe,"k=     f=%4d", glaeser[rotaries[SW_MENU].Value].Gewicht );
  } else if ( rotary_select == SW_MENU && time < 3 ) {
    sprintf(ausgabe,"k=%-3d  f=" , rotaries[SW_KORREKTUR].Value);
  } else {
    sprintf(ausgabe,"k=%-3d  f=%4d", rotaries[SW_KORREKTUR].Value, glaeser[rotaries[SW_MENU].Value].Gewicht );
  }
  u8g2.print(ausgabe);

  u8g2.sendBuffer();
}

void processHandbetrieb(void)
{
  if ( modus != MODE_HANDBETRIEB ) {
     modus = MODE_HANDBETRIEB;
     winkel = winkel_min;          // Hahn schliessen
     servo_aktiv = 0;                        // Servo-Betrieb aus
     servo.write(winkel);
     rotary_select = SW_WINKEL;
  }

//  pos = (map(analogRead(poti_pin), 0, 4095, 100, 0));
  pos = rotaries[SW_WINKEL].Value;
  gewicht = ((((int(SCALE_READAVERAGE(SCALE_READS))) - tara_raw) / faktor) - tara);
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    servo_aktiv = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    servo_aktiv = 0;
  }
  
  if (servo_aktiv == 1) {
    winkel = ((winkel_max * pos) / 100);
  } else { 
    winkel = winkel_min;
  }
  servo.write(winkel);
  
  #ifdef isDebug
//    Serial.print(SCALE_READAVERAGE(SCALE_READS));     // erneutes Lesen der Waage verfälscht die Debug-Ausgabe!
    Serial.print(" Tara_raw:");
    Serial.print(tara_raw);
    Serial.print(" Faktor ");
    Serial.print(faktor);
    Serial.print(" Gewicht ");
    Serial.print(gewicht);
    Serial.print(" Winkel ");
    Serial.print(winkel);
    Serial.print(" servo_aktiv ");
    Serial.println(servo_aktiv);
  #endif
  
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_courB24_tf);
  u8g2.setCursor(10, 42);
  sprintf(ausgabe,"%5dg", gewicht);
  u8g2.print(ausgabe);

  u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
  u8g2.drawGlyph(0, 40, (servo_aktiv==1)?0x45:0x44 );

  u8g2.setFont(u8g2_font_courB12_tf);
  u8g2.setCursor(0, 11);
  sprintf(ausgabe,"W=%-3d    %3d%%", winkel, pos);
  u8g2.print(ausgabe);
  u8g2.setCursor(0, 64);
  u8g2.print("Handbetrieb");

  u8g2.sendBuffer();
}

void setup()
{
  // enable internal pull downs for digital inputs 
  pinMode(button_start_pin, INPUT_PULLDOWN);
  pinMode(button_stop_pin, INPUT_PULLDOWN);
  pinMode(switch_betrieb_pin, INPUT_PULLDOWN);
  pinMode(switch_setup_pin, INPUT_PULLDOWN);
#ifndef HARDWARE_V1
  pinMode(vext_ctrl_pin, INPUT_PULLDOWN);
#endif
  pinMode(LED_BUILTIN, OUTPUT);

  // Rotary
  pinMode(outputA,INPUT);
  pinMode(outputB,INPUT);
  aLastState = digitalRead(outputA);
  //attachInterrupt(outputA, rotaryCheck, CHANGE);
  pinMode(outputSW, INPUT_PULLUP);
  attachInterrupt(outputSW, isr1, FALLING);
  attachInterrupt(outputA, isr2, CHANGE);

// switch Vcc / GND on normal pins for convenient wiring
// output is 3.3V for VCC
  digitalWrite (switch_vcc_pin, HIGH); 
  digitalWrite (button_start_vcc_pin, HIGH); 
  digitalWrite (button_stop_vcc_pin, HIGH); 
  
//  pinMode (_GND, OUTPUT);     // turn on GND pin first (important!)
  // turn on VCC power
  pinMode (switch_vcc_pin, OUTPUT);
  pinMode (button_start_vcc_pin, OUTPUT);
  pinMode (button_stop_vcc_pin, OUTPUT);
  // short delay to let chip power up
  delay (100); 
  
  u8g2.begin();
  // print Boot Screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB24_tf);
  u8g2.setCursor(20, 43);  
  u8g2.print("BOOT");
  u8g2.sendBuffer();

  Serial.begin(115200);
  while (!Serial) {
  }

  scale.begin(hx711_dt_pin, hx711_sck_pin);
  if (scale.wait_ready_timeout(1000)) {
    scale.power_up();
    waage_vorhanden = 1;
  } else {
    u8g2.clearBuffer();
    u8g2.setCursor( 14, 24);    u8g2.print("Keine");
    u8g2.setCursor( 6, 56);     u8g2.print("Waage!");
    u8g2.sendBuffer();
#ifdef isDebug
    Serial.println("Keine Waage!");
#endif
    delay(1000);
  }
  
//  servo.attach(servo_pin, 750, 2500);
  servo.attach(servo_pin);
  servo.write(winkel_min);
  
  getPreferences();
}


void loop()
{
  rotating = true;     // debounce Management
  
  // Setup Menu 
  if ((digitalRead(switch_setup_pin)) == HIGH)
    processSetup();

  // Automatik-Betrieb 
  if ((digitalRead(switch_betrieb_pin)) == HIGH)
    processAutomatik();

  // Handbetrieb 
  if ((digitalRead(switch_betrieb_pin) == LOW)
      && (digitalRead(switch_setup_pin) == LOW))
    processHandbetrieb();
}

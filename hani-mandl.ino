/*
  Abfuellwaage Version 0.2.1
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

#define SETUP       0
#define BETRIEB     1
#define HANDBETRIEB 2

#define SCALE_READS 3     // Parameter für scale.read_average()

Servo servo;
HX711 scale;
Preferences preferences;

// ** Definition der pins 
// ----------------------

// OLED
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
// fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Vext control pin
const int vext_ctrl_pin = 21;

// Servo
const int servo_pin = 2;

// 3x Schalter Ein 1 - Aus - Ein 2
const int switch_betrieb_pin = 23;
const int switch_vcc_pin = 19;        // <- Vcc 
const int switch_setup_pin = 22;

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

int pos;
int gewicht;
int tara;             // Tara für das Standard-Glas, für Automatikmodus
int tara_glas;        // Tara für das aktuelle Glas, falls Glasgewicht abweicht
int tara_raw;         // Tara der leeren Waage
int gewicht_raw;      // Gewicht Raw-Wert, nur zur Kalkulation tara_raw
float faktor;         // Skalierungsfaktor für Werte der Waage
int faktor2;          // faktor * 10000 für Preferences
int fmenge;
int korrektur;
int autostart;
int winkel;
int winkel_min = 0;
int winkel_max = 155;
int winkel_dosier_min = 45;
float fein_dosier_gewicht = 60; // float wegen Berechnung des Schliesswinkels
int i;
int a;
char ausgabe[30]; // Fontsize 12 = 13 Zeichen maximal in einer Zeile
int modus = -1;   // Bei Modus-Wechsel den Servo auf Minimum fahren
int autofill = 0; // Für Automatikmodus - System ein/aus?

void getPreferences(void) {
  // EEPROM //
  preferences.begin("EEPROM", false);       //faktor und tara aus eeprom lesen
  faktor2 = preferences.getUInt("faktor2", 0);

  #ifdef isDebug
    if (faktor2 == 0) {
      Serial.println("Waage ist nicht kalibiert!");
      for (int i=0; i < 200; i++) {
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  #endif
  
  faktor = (faktor2 / 10000.00);
  tara      = preferences.getUInt("tara", 0);
  tara_raw  = preferences.getUInt("tara_raw", 0);
  fmenge    = preferences.getUInt("fmenge", 0);
  korrektur = preferences.getUInt("korrektur", 0);
  autostart = preferences.getUInt("autostart", 0);
  preferences.end();

#ifdef isDebug
  Serial.println("Preferences:");
  Serial.print("faktor = ");    Serial.println(faktor);
  Serial.print("tara = ");      Serial.println(tara);
  Serial.print("tara_raw = ");  Serial.println(tara_raw);
  Serial.print("fmenge = ");    Serial.println(fmenge);
  Serial.print("korrektur = "); Serial.println(korrektur);
  Serial.print("autostart = "); Serial.println(autostart);
#endif
}

void setupTara(void) {
  u8g2.setCursor(0, 8);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    tara = ((int(scale.read_average(30)) - tara_raw) / faktor);
    u8g2.setCursor(100, 12);
    u8g2.print("OK");
    u8g2.sendBuffer();
    delay(2000);
    preferences.begin("EEPROM", false);
    preferences.putUInt("tara", tara);
    preferences.end();
  }
}

void setupCalibration(void) {
  u8g2.setCursor(0, 22);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    delay(300);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);
    u8g2.print("Bitte 500g");
    u8g2.setCursor(10, 28);
    u8g2.print("aufstellen");
    u8g2.setCursor(10, 44);
    u8g2.print("& mit Start");
    u8g2.setCursor(10, 60);
    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    i = 1;
    while (i > 0) {
      if ((digitalRead(button_start_pin)) == HIGH) {
        gewicht_raw = (int(scale.read_average(30)));
        delay(2000);
        i = 0;
      }
    }
    
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);
    u8g2.print("Bitte Waage");
    u8g2.setCursor(10, 28);
    u8g2.print("leeren");
    u8g2.setCursor(10, 44);
    u8g2.print("& mit Start");
    u8g2.setCursor(10, 60);
    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    i = 1;
    while (i > 0) {
      if ((digitalRead(button_start_pin)) == HIGH) {
        tara_raw = (int(scale.read_average(30)));
        delay(2000);
        i = 0;
        faktor = ((gewicht_raw - tara_raw) / 500.000);

        preferences.begin("EEPROM", false);  // faktor und tara ins eeprom schreiben
        preferences.putUInt("faktor2", (faktor * 10000));
        preferences.putUInt("tara_raw", tara_raw);
        preferences.end();
      }
    }
  }
}

void setupKorrektur(void) {
  u8g2.setCursor(0, 36);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(300);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = (map(analogRead(poti_pin), 0, 4095, 10, -50));
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print("Korrektur");
      u8g2.setCursor(40, 28);
      u8g2.print(pos);
      u8g2.sendBuffer();
      
      if ((digitalRead(button_start_pin)) == HIGH) {
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
  }
}

void setupFuellmenge(void) {
  u8g2.setCursor(0, 50);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(200);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = (map(analogRead(poti_pin), 0, 4095, 1, 4));
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print(" 500g");
      u8g2.setCursor(10, 28);
      u8g2.print(" 250g");
      u8g2.setCursor(10, 44);
      u8g2.print("  50g");
      u8g2.setCursor(10, 60);
      u8g2.print("  20g");
      u8g2.sendBuffer();
      
      if (pos == 1) {
        u8g2.setCursor(0, 12);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          fmenge = 500;
          u8g2.setCursor(100, 12);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
        }
      }
      
      if (pos == 2) {
        u8g2.setCursor(0, 28);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          fmenge = 250;
          u8g2.setCursor(100, 28);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
        }
      }
      
      if (pos == 3) {
        u8g2.setCursor(0, 44);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          fmenge = 50;
          u8g2.setCursor(100, 44);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
        }
      }
      
      if (pos == 4) {
        u8g2.setCursor(0, 60);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          fmenge = 20;
          u8g2.setCursor(100, 60);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
        }
      }
    }
  }
}

void setupAutostart(void) {
  u8g2.setCursor(0, 64);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(200);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = (map(analogRead(poti_pin), 0, 4095, 1, 2));
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print("Auto EIN");
      u8g2.setCursor(10, 28);
      u8g2.print("Auto AUS");
      u8g2.sendBuffer();
      
      if (pos == 1) {
        u8g2.setCursor(0, 12);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          autostart = 1;
          u8g2.setCursor(105, 12);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("autostart", autostart);
          preferences.end();
        }
      }
      
      if (pos == 2) {
        u8g2.setCursor(0, 28);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          autostart = 2;
          u8g2.setCursor(105, 28);
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
  }
}

void processSetup(void) {
  if ( modus != SETUP ) {
     modus = SETUP;
     winkel = winkel_min;          // Hahn schliessen
     a=0;                          // Servo-Betrieb aus
     servo.write(winkel);
  }
  
  pos = (map(analogRead(poti_pin), 0, 4095, 1, 5));

  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  u8g2.setCursor(10, 8);
  u8g2.print("Tara");
  u8g2.setCursor(10, 22);
  u8g2.print("Kalibrieren");
  u8g2.setCursor(10, 36);
  u8g2.print("Korrektur");
  u8g2.setCursor(10, 50);
  u8g2.print("Fuellmenge");
  u8g2.setCursor(10, 64);
  u8g2.print("Autostart");

  // Tara 
  if (pos == 1)
    setupTara();
    
  // Kalibrieren 
  if (pos == 2)
    setupCalibration();

  // Korrektur 
  if (pos == 3)
    setupKorrektur();
    
  // Füllmenge 
  if (pos == 4)
    setupFuellmenge();

  // Autostart 
  if (pos == 5)
    setupAutostart();

  u8g2.sendBuffer();
}

void processBetrieb(void)
{
  int zielgewicht;
  
  if ( modus != BETRIEB ) {
     modus = BETRIEB;
     winkel = winkel_min;          // Hahn schliessen
     a=0;                          // Servo-Betrieb aus
     servo.write(winkel);
     autofill = 0;
     tara_glas = 0;
  }

  pos = (map(analogRead(poti_pin), 0, 4095, 100, 0));
  gewicht = ((((int(scale.read_average(SCALE_READS))) - tara_raw) / faktor) - tara);

  if ((autofill == 1) && (gewicht <= 5) && (gewicht >= -5) && (a == 0)) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB24_tf);
    u8g2.setCursor(15, 43);
    u8g2.print("START");
    u8g2.sendBuffer();
    // kurz warten und prüfen ob das gewicht nicht nur eine zufällige Schwankung war 
    delay(1500);  
    gewicht = ((((int(scale.read_average(SCALE_READS))) - tara_raw) / faktor) - tara);

    if ((gewicht <= 5) && (gewicht >= -5)) {
      tara_glas = gewicht;
      a = 1;
    }
  }

  zielgewicht = fmenge + korrektur + tara_glas;

  // Glas entfernt -> Servo schliessen
  if (gewicht < -20) {
    winkel = winkel_min;
    a = 0;
    tara_glas = 0;
    if ( autostart != 1 ) {
      autofill = 0;
    }
  }
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    autofill = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    winkel = winkel_min;
    a = 0;
    autofill = 0;
    tara_glas = 0;
  }

  // Füll-Automatik ohne Autostart ist aktiviert, Glas ist teilweise gefüllt
  // Füllvorgang fortsetzen
  if ((autofill == 1) && (gewicht >= 0) && (autostart != 1)) {
    a = 1;
  }
  
  if (a == 1) {
    winkel = ((winkel_max * pos) / 100);
  }
  
  if ((a == 1) && (fmenge - (gewicht - korrektur - tara_glas) <= fein_dosier_gewicht)) {
    winkel = ( ((winkel_max * pos) / 100)
        * ( (fmenge - (gewicht - korrektur - tara_glas)) / fein_dosier_gewicht) );
  }
  
  if ((a == 1) && (winkel <= winkel_dosier_min)) {
    winkel = winkel_dosier_min;
  }
  
  if ((a == 1) && ((gewicht - korrektur - tara_glas) >= fmenge)) {
    winkel = winkel_min;
    a = 0;
    if ( autostart != 1 ) {
      autofill = 0;
    }
  }
  
  servo.write(winkel);
  
  #ifdef isDebug
    Serial.print(scale.read_average(SCALE_READS));
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
  sprintf(ausgabe,"W=%-3d %2s %3d%%", winkel, (autostart==1)?"AS":"  ", pos);
  u8g2.print(ausgabe);

  u8g2.setCursor(0, 64);
  sprintf(ausgabe,"k=%-3d   f=%3d", korrektur, fmenge);
  u8g2.print(ausgabe);

  u8g2.sendBuffer();
}

void processHandbetrieb(void)
{
  if ( modus != HANDBETRIEB ) {
     modus = HANDBETRIEB;
     winkel = winkel_min;          // Hahn schliessen
     a=0;                          // Servo-Betrieb aus
     servo.write(winkel);
  }

  pos = (map(analogRead(poti_pin), 0, 4095, 100, 0));
  gewicht = ((((int(scale.read_average(SCALE_READS))) - tara_raw) / faktor) - tara);
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    a = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    (winkel = winkel_min);
    a = 0;
  }
  
  if (a == 1) {
    winkel = ((winkel_max * pos) / 100);
  }

  servo.write(winkel);
  
  #ifdef isDebug
    Serial.print(scale.read_average(SCALE_READS));
    Serial.print(" Tara_raw:");
    Serial.print(tara_raw);
    Serial.print(" Faktor ");
    Serial.print(faktor);
    Serial.print(" Gewicht ");
    Serial.print(gewicht);
    Serial.print(" Winkel ");
    Serial.print(winkel);
    Serial.print(" a ");
    Serial.println(a);
  #endif
  
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_courB24_tf);

  u8g2.setCursor(10, 42);
  sprintf(ausgabe,"%5dg", gewicht);
  u8g2.print(ausgabe);

  u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
  u8g2.drawGlyph(0, 40, (a==1)?0x45:0x44 );

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
  pinMode(vext_ctrl_pin, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);

  // switch Vcc / GND on normal pins for convenient wiring
  // output 5V for VCC
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
  scale.power_up();

//  servo.attach(servo_pin, 750, 2500);
  servo.attach(servo_pin);
  servo.write(winkel_min);
  
  getPreferences();
}


void loop()
{
  if ((digitalRead(switch_setup_pin)) == HIGH)
    processSetup();

  // Betrieb 
  if ((digitalRead(switch_betrieb_pin)) == HIGH)
    processBetrieb();

  // Handbetrieb 
  if ((digitalRead(switch_betrieb_pin) == LOW)
      && (digitalRead(switch_setup_pin) == LOW))
    processHandbetrieb();
}

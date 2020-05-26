/*
  Abfuellwaage Version 0.1.4
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
  2020-05 Marc Junker     | Erweiterung von Poti auf Rotary Encoder; alle Serial.print´s in #ifdef eingeschlossen; "Start" delay verkürzt
                            glas von const in Array geändert
  
                            
  This code is in the public domain.
  
  
  Hinweise zur Hardware
  ---------------------
  - bei allen digitalen Eingänge sind interne pull downs aktiviert, keine externen-Widerständen nötig! 
*/

//const int VERSION 123

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>      /* aus dem Bibliotheksverwalter */
#include <HX711.h>        /* https://github.com/bogde/HX711 */
#include <ESP32_Servo.h>  /* https://github.com/jkb-git/ESP32Servo */
#include <Preferences.h>  /* aus dem BSP von expressif */

//#define isDebug 

Servo servo;
HX711 scale;
Preferences preferences;

// ** Definition der pins 
// ----------------------

// OLED
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
// fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const int Setup = 1;
const int Betrieb = 2;
const int Handbetrieb = 3;
int Betriebsmodus;

// Rotary
#define outputA 33
#define outputB 26
#define outputSW 32
#define button_rotary 32

volatile int counter_pos = 50; 
volatile int counter_k = 6;
volatile int counter_glas = 3;
volatile int counter_poti = 0;
int aState;
int aLastState;  

// Servo
const int servo_pin = 2;

// 3x Schalter Ein 1 - Aus - Ein 2
const int switch_betrieb_pin = 19;
const int switch_vcc_pin = 22;        // <- Vcc 
const int switch_setup_pin = 21;

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

// wir verwenden auch den pin fuer LED_BUILTIN!
// es ist pin 25 fuer den Heltec WiFi Kit 32 


const char* gewicht_char = "";

int pos;
int gewicht;
int menu_kalib;
int menu_korrek;
int tara;
int tara_raw;
int gewicht_raw;
float faktor;
int faktor2;
int fmenge;
int korrektur;
int autostart;
int winkel;
int winkel_min = 0;
int winkel_max = 110;
int winkel_dosier_min = 30;
float fein_dosier_gewicht = 60;
int i;
int u;
int a;
int z;

const int Anzahl_Glaeser = 6;
int glas[Anzahl_Glaeser] = {125, 250, 440, 500, 535, 700};
const int glas4 = 535;
const int glas3 = 500;
const int glas2 = 440;
const int glas1 = 250;
int tara_glas[Anzahl_Glaeser] = {100, 200, 300, 400, 500, 600};
//int tara_raw_glas[Anzahl_Glaeser];

void IRAM_ATTR isr2() { 
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  if (digitalRead(outputSW) == HIGH) {  
    if (digitalRead(button_stop_pin) == HIGH) {
       if (aState != aLastState){     
         if (digitalRead(outputB) != aState) { 
           counter_glas = mod((counter_glas+1) , Anzahl_Glaeser*10);
         }
         else {
           counter_glas = mod((counter_glas-1) , Anzahl_Glaeser*10);
         }
      } 
    }
   else {
     if (aState != aLastState){     
       if (digitalRead(outputB) != aState) { 
          if (Betriebsmodus != Setup) {
            if (counter_pos>0) {counter_pos--;}
          }
          else {
            if (counter_poti >0) {counter_poti -=2;}
          }
       }


           
          else {
             if (Betriebsmodus != Setup) {
               if (counter_pos<200) { counter_pos++;}
             }
             else {
              if (counter_poti <50) {counter_poti +=2;}
             }
          
        }
      }
    } 
  }
  else {
    if (aState != aLastState){     
       if (digitalRead(outputB) != aState) { 
         if (counter_k<100) { counter_k++;}
       } 
       else {      
         if (counter_k>-100) {counter_k--;}
       }
     }
   }   
 aLastState = aState; // Updates the previous state of the outputA with the current state
}


// eingene Modulo-Funktion, die auch mit negativen Zahlen korrekt klarkommt 
int mod( int x, int y ){
   return x<0 ? ((x+1)%y)+y-1 : x%y;
}


void print2serial(String displayname, int value) {
  Serial.print(displayname);
  Serial.println(value);
}

void print2serial(String displayname, float value) {
  Serial.print(displayname);
  Serial.println(value);
}

void getPreferences(void) {
  // EEPROM //
  preferences.begin("EEPROM", false);       //faktor und tara aus eeprom lesen
  faktor2 = preferences.getUInt("faktor2", 0);

  #ifdef isDebug
    if (faktor2 == 0) {
      //Serial.println("Waage ist nicht kalibiert!");
      for (int i=0; i < 200; i++) {
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  #endif
  
  faktor = (faktor2 / 10000.00);
  tara_glas[0] = preferences.getUInt("tara0", 0);
   tara_glas[1] = preferences.getUInt("tara1", 0);
    tara_glas[2] = preferences.getUInt("tara2", 0);
     tara_glas[3] = preferences.getUInt("tara3", 0);
      tara_glas[4] = preferences.getUInt("tara4", 0);
       tara_glas[5] = preferences.getUInt("tara5", 0);
  tara_raw = preferences.getUInt("tara_raw", 0);
  counter_glas = preferences.getUInt("fmenge", 0);
  counter_k = preferences.getUInt("korrektur", 0);
  autostart = preferences.getUInt("autostart", 0);
  counter_pos = preferences.getUInt("pos", 0);

  //print2serial("faktor = ", faktor);
  //print2serial("tara_raw = ", tara_raw);
  //print2serial("tara = ", tara);
  preferences.end();
}

void setupTara(void) {
  u8g2.setCursor(0, 8);
  u8g2.print("*");
  
  //if ((digitalRead(button_start_pin)) == HIGH) {
    if ((digitalRead(button_rotary)) == LOW) {
    
    tara_glas[(int)(counter_glas/10)] = ((int(scale.read_average(10)) - tara_raw) / faktor);
    u8g2.setCursor(100, 12);
    u8g2.print("OK");
    u8g2.sendBuffer();
    delay(2000);
    preferences.begin("EEPROM", false);
    switch ((int)(counter_glas/10)) {
      case 1:
         preferences.putUInt("tara1", tara_glas[(int)(counter_glas/10)]);
         break;
      case 2:
         preferences.putUInt("tara2", tara_glas[(int)(counter_glas/10)]);
         break;
      case 3:
         preferences.putUInt("tara3", tara_glas[(int)(counter_glas/10)]);
         break;
      case 4:
         preferences.putUInt("tara4", tara_glas[(int)(counter_glas/10)]);
         break;
      case 5:
         preferences.putUInt("tara5", tara_glas[(int)(counter_glas/10)]);
         break;
      case 6:
         preferences.putUInt("tara6", tara_glas[(int)(counter_glas/10)]);
}
  
    preferences.end();
  }
}

void setupCalibration(void) {
  u8g2.setCursor(0, 22);
  u8g2.print("*");
  
//  if ((digitalRead(button_start_pin)) == HIGH) {
  if ((digitalRead(button_rotary)) == LOW) {
    i = 1;
    delay(300);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);
    u8g2.print("Bitte 500g");
    u8g2.setCursor(10, 28);
    u8g2.print("aufstellen");
    u8g2.setCursor(10, 44);
    u8g2.print("& mit OK");
    u8g2.setCursor(10, 60);
    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    while (i > 0) {
      //if ((digitalRead(button_start_pin)) == HIGH) {
      if ((digitalRead(button_rotary)) == LOW) {
        gewicht_raw = (int(scale.read_average(10)));
        delay(2000);
        i = 0;
      }
    }
    
    i = 1;
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);
    u8g2.print("Bitte Waage");
    u8g2.setCursor(10, 28);
    u8g2.print("leeren");
    u8g2.setCursor(10, 44);
    u8g2.print("& mit OK");
    u8g2.setCursor(10, 60);
    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    while (i > 0) {
     // if ((digitalRead(button_start_pin)) == HIGH) {
      if ((digitalRead(button_rotary)) == LOW) {
        tara_raw = (int(scale.read_average(10)));
        delay(2000);
        i = 0;
        faktor = ((gewicht_raw - tara_raw) / 500.000);

        preferences.begin("EEPROM", false);  // faktor und tara ins eeprom schreiben
        preferences.putUInt("faktor2", (faktor * 10000));
        preferences.putUInt("tara_raw", tara_raw);
        preferences.end();
        counter_poti = 0;
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
      pos = (map(analogRead(poti_pin), 0, 4095, -50, 10));
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

void setupGlas1(void)
{
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  u8g2.setCursor(10, 12);
  u8g2.print("Leeres Glas");
  u8g2.setCursor(10, 28);
  u8g2.print("aufstellen und");
  u8g2.setCursor(10, 44);
  u8g2.print("OK druecken.");
  u8g2.sendBuffer();
  while ((digitalRead(button_start_pin)) == LOW) {};
  setupTara();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  u8g2.setCursor(10, 12);
  u8g2.print("Volles Glas");
  u8g2.setCursor(10, 28);
  u8g2.print("aufstellen und");
  u8g2.setCursor(10, 44);
  u8g2.print("OK druecken.");
  u8g2.sendBuffer();
  
  delay(1000);
  while ((digitalRead(button_start_pin)) == LOW) {};
  fmenge = ((((int(scale.read())) - tara_raw) / faktor) - tara);
  
          u8g2.setCursor(100, 12);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
  u8g2.clearBuffer();
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
      u8g2.print(" "+(String)glas4+"g");
      u8g2.setCursor(10, 28);
      u8g2.print(" "+(String)glas3+"g");
      u8g2.setCursor(10, 44);
      u8g2.print(" "+(String)glas2+"g");
      u8g2.setCursor(10, 60);
      //u8g2.print(" "+(String)glas1+"g");
      u8g2.print(" Auto");
      u8g2.sendBuffer();
      
      if (pos == 1) {
        u8g2.setCursor(0, 12);
        u8g2.print("*");
        u8g2.sendBuffer();
        
        if ((digitalRead(button_start_pin)) == HIGH) {
          fmenge = glas4;
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
          fmenge = glas3;
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
          fmenge = glas2;
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
          setupGlas1();
          /*
          fmenge = glas1;
          u8g2.setCursor(100, 60);
          u8g2.print("OK");
          u8g2.sendBuffer();
          delay(2000);
          i = 0;
          preferences.begin("EEPROM", false);
          preferences.putUInt("fmenge", fmenge);
          preferences.end();
          */
        }
      }
    }
  }
}

void setupAutostart(void) {
  u8g2.setCursor(0, 36);
  u8g2.print("*");
  
  //if ((digitalRead(button_start_pin)) == HIGH) {
  if ((digitalRead(button_rotary)) == LOW) {
    i = 1;
    delay(200);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      //pos = (map(analogRead(poti_pin), 0, 4095, 1, 2));
      pos = map(counter_poti, 0, 50, 1, 2);
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
        
  //      if ((digitalRead(button_start_pin)) == HIGH) {
        if ((digitalRead(button_rotary)) == LOW) {
          autostart = 1;
          u8g2.setCursor(105, 12);
          u8g2.print("OK");
          u8g2.sendBuffer();
          counter_poti = 0;
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
        
//        if ((digitalRead(button_start_pin)) == HIGH) {
        if ((digitalRead(button_rotary)) == LOW) { 
          autostart = 2;
          u8g2.setCursor(105, 28);
          u8g2.print("OK");
          u8g2.sendBuffer();
          counter_poti = 0;
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
  //pos = (map(analogRead(poti_pin), 0, 4095, 1, 5));
  pos = map(counter_poti, 0, 50, 1, 3);
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  u8g2.setCursor(10, 8);
  u8g2.print("Tara");
  u8g2.setCursor(10, 22);
  u8g2.print("Kalibrieren");
  u8g2.setCursor(10, 36);
  //u8g2.print("Korrektur");
  //u8g2.setCursor(10, 50);
  //u8g2.print("Fuellmenge");
  //u8g2.setCursor(10, 64);
  u8g2.print("Autostart");

  // Tara 
  if (pos == 1)
    setupTara();
    
  // Kalibrieren 
  if (pos == 2)
    setupCalibration();

  // Korrektur 
  //if (pos == 3)
  //  setupKorrektur();
    
  // Füllmenge 
  //if (pos == 4)
  //  setupFuellmenge();

  // Autostart 
  if (pos == 3)
    setupAutostart();

  u8g2.sendBuffer();
  a = 0;
}

void processBetrieb(void)
{
  //pos = (map(analogRead(poti_pin), 0, 4095, 0, 100));
  int pos_tmp = pos;
  pos = counter_pos / 2;
  if (pos != pos_tmp) {
     preferences.begin("EEPROM", false);
     preferences.putUInt("pos", counter_pos);
     preferences.end();
  }
  gewicht = ((((int(scale.read())) - tara_raw) / faktor) - tara_glas[(int)(counter_glas/10)]);
  int fmenge_tmp = fmenge;
  fmenge = glas[(int)(counter_glas/10)];
  if (fmenge != fmenge_tmp) {                // preferences in Interrupt Funkt
       preferences.begin("EEPROM", false);
       preferences.putUInt("fmenge", counter_glas);
       preferences.end();
  }
  
  if ((autostart == 1) && (gewicht <= 5) && (gewicht >= -5) && (a == 0)) {
    delay(1000);
    
    if ((gewicht <= 5) && (gewicht >= -5)) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_courB24_tf);
      u8g2.setCursor(20, 43);
      u8g2.print("START");
      u8g2.sendBuffer();
      delay(500);
      a = 1;
    }
  }
  
  if ((autostart == 1) && (gewicht < -20)) {
    winkel = winkel_min;
    a = 0;
  }
  
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
  
  if ((a == 1) && (fmenge + korrektur - gewicht <= fein_dosier_gewicht)) {
    winkel = ((((winkel_max * pos) / 100)
        * ((fmenge + korrektur - gewicht) / fein_dosier_gewicht)));
  }
  
  if ((a == 1) && (winkel <= winkel_dosier_min)) {
    winkel = winkel_dosier_min;
  }
  
  if ((a == 1) && ((gewicht - korrektur) >= fmenge)) {
    winkel = winkel_min;
    a = 0;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    (winkel = winkel_min);
    a = 0;
  }
  
  servo.write(winkel);
  float y = ((fmenge + korrektur - gewicht) / fein_dosier_gewicht);
  //Serial.println(y);
  
  #ifdef isDebug
    Serial.print(scale.read_average(3));
    Serial.print(" Tara_raw:");
    Serial.print(tara_raw);
    Serial.print(" Faktor ");
    Serial.print(faktor);
    Serial.print(" Gewicht ");
    Serial.println(gewicht);
  #endif

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB24_tf);
  
  if (gewicht < 100) {
    u8g2.setCursor(55, 43);
  }
  
  if (gewicht < 10) {
    u8g2.setCursor(75, 43);
  }
  
  if (gewicht >= 100) {
    u8g2.setCursor(35, 43);
  }
  
  if (gewicht >= 1000) {
    u8g2.setCursor(15, 43);
  }
  
  if (gewicht < 0) {
    u8g2.setCursor(55, 43);
  }
  
  if (gewicht <= -10) {
    u8g2.setCursor(35, 43);
  }
  
  if (gewicht <= -100) {
    u8g2.setCursor(15, 43);
  }
  
  u8g2.print(gewicht);
  u8g2.setCursor(95, 43);
  u8g2.print("g");
  u8g2.setFont(u8g2_font_courB14_tf);
  
  #ifdef isDebug
    u8g2.setCursor(0,13);
    u8g2.print("t=");
    u8g2.setCursor(24,13);
    u8g2.print(tara);
  #endif
  
  u8g2.setCursor(0, 13);
  u8g2.print("W=");
  u8g2.setCursor(24, 13);
  u8g2.print(winkel);
  
  if (autostart == 1) {
    u8g2.setCursor(58, 13);
    u8g2.print("AS");
  }
  //marc
  int korrektur_tmp = korrektur;
  korrektur = (int)(counter_k / 5);
  if (korrektur != korrektur_tmp) {
     preferences.begin("EEPROM", false);
     preferences.putUInt("korrektur", counter_k);
     preferences.end();
  }
  //u8g2.setColorIndex(0);
  u8g2.setCursor(0, 64);
  u8g2.print("k=");
  //u8g2.setColorIndex(1);
  u8g2.setCursor(24, 64);
  u8g2.print(korrektur);
  u8g2.setCursor(73, 64);
  u8g2.print("f=");
  u8g2.setCursor(97, 64);
  u8g2.print(fmenge);
  if (pos < 100) {
    u8g2.setCursor(98, 13);
  }
  
  if (pos < 10) {
    u8g2.setCursor(108, 13);
  }
  
  if (pos >= 100) {
    u8g2.setCursor(88, 13);
  }
  
  u8g2.print(pos);
  u8g2.setCursor(120, 13);
  u8g2.print(char(37));
  u8g2.sendBuffer();
}

void processHandbetrieb(void)
{
  //checkRotary();
  //pos = (map(analogRead(poti_pin), 0, 4095, 0, 100));
  pos = counter_pos / 2;
  gewicht = ((((int(scale.read())) - tara_raw) / faktor) - tara);
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    a = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    (winkel = winkel_min);
    a = 0;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    (winkel = winkel_min);
  }
  
  if (a == 1) {
    winkel = ((winkel_max * pos) / 100);
  }
  
  servo.write(winkel);
  
  #ifdef isDebug
    Serial.print(scale.read_average(3));
    Serial.print(" Tara_raw:");
    Serial.print(tara_raw);
    Serial.print(" Faktor ");
    Serial.print(faktor);
    Serial.print(" Gewicht ");
    Serial.println(gewicht);
  #endif
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB24_tf);
  
  if (gewicht < 100) {
    u8g2.setCursor(55, 43);
  }
  
  if (gewicht < 10) {
    u8g2.setCursor(75, 43);
  }
  
  if (gewicht >= 100) {
    u8g2.setCursor(35, 43);
  }
  
  if (gewicht >= 1000) {
    u8g2.setCursor(15, 43);
  }
  
  if (gewicht < 0) {
    u8g2.setCursor(55, 43);
  }
  
  if (gewicht <= -10) {
    u8g2.setCursor(35, 43);
  }
  
  if (gewicht <= -100) {
    u8g2.setCursor(15, 43);
  }
  
  u8g2.print(gewicht);
  u8g2.setCursor(95, 43);
  u8g2.print("g");
  u8g2.setFont(u8g2_font_courB14_tf);
  u8g2.setCursor(0, 13);
  u8g2.print("W=");
  u8g2.setCursor(24, 13);
  u8g2.print(winkel);
  
  if (pos < 100) {
    u8g2.setCursor(98, 13);
  }
  
  if (pos < 10) {
    u8g2.setCursor(108, 13);
  }
  
  if (pos >= 100) {
    u8g2.setCursor(88, 13);
  }
  
  u8g2.print(pos);
  u8g2.setCursor(120, 13);
  u8g2.print(char(37));
  u8g2.sendBuffer();
//a=0;
}


void setup()
{
  u8g2.begin();
  u8g2.clearBuffer();
// 'undefined', 128x64px
/*const unsigned char myBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xe0, 0x30, 0x18, 0x08, 0x08, 0x18, 0xf0, 0x60, 0x30, 0x10, 0x10, 0xb0, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x3c, 0x04, 0x0c, 0x18, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xe0, 0x3e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x03, 0x06, 0x0c, 0x18, 0x98, 0x0c, 0x06, 0x02, 0x03, 0x01, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x80, 0xf0, 0x1f, 0x03, 0xfc, 0x00, 0xe0, 0x38, 0x0e, 0x03, 0xfe, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x80, 0xe0, 0x3f, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x02, 0xff, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xf2, 
  0x0e, 0x05, 0x04, 0x04, 0x04, 0xc4, 0x70, 0x18, 0x08, 0x04, 0x04, 0x04, 0x04, 0x82, 0x42, 0xb2, 
  0x7e, 0x04, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf8, 0xc6, 0x30, 0x18, 0x0c, 0x02, 0x02, 0x02, 0xfc, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x60, 0x1c, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 
  0x80, 0xe0, 0x30, 0x10, 0x10, 0x10, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x80, 0xe0, 0x30, 0x10, 
  0x30, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0xff, 0x0c, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x07, 0x0c, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x10, 0x18, 0x08, 0x04, 0x03, 0x01, 0x00, 0x1f, 
  0x10, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x38, 
  0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x01, 0x00, 0x00, 
  0x1f, 0x10, 0x10, 0x10, 0x18, 0x1f, 0x31, 0x20, 0x20, 0x00, 0x00, 0x7f, 0x07, 0x00, 0x00, 0x00, 
  0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x43, 0x41, 0x21, 0x1f, 0x01, 0x1f, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x38, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 
  0x3c, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xf0, 0x30, 0x10, 0xb0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0xc0, 
  0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x2c, 0x66, 0xc3, 
  0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xc0, 0x60, 0x23, 0x16, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xfe, 0x01, 0x01, 0x03, 0x02, 0x0e, 0x08, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 
  0x41, 0x41, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xc0, 0x00, 0x80, 0xe0, 
  0x38, 0x04, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0x03, 0x05, 0x19, 0x60, 0x00, 0x08, 0x14, 0x12, 0x12, 0x0e, 0x00, 0x03, 
  0x0e, 0x18, 0x10, 0x00, 0x00, 0x0f, 0x09, 0x1f, 0x10, 0x00, 0x00, 0x1f, 0x02, 0x03, 0x01, 0x00, 
  0x00, 0x00, 0xc3, 0x76, 0x1c, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x01, 0x03, 0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 
  0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x06, 0x1b, 0x10, 0x10, 0x10, 0x18, 0x08, 0x04, 0x07, 0x00, 0x00, 0x00, 0x0c, 0x07, 
  0x08, 0x18, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x00, 0x00, 0x00, 0x07, 0x0c, 0x07, 0x00, 
  0x00, 0x00, 0x00, 0x0c, 0x12, 0x21, 0x10, 0x19, 0x06, 0x00, 0x10, 0x00, 0x03, 0x10, 0x1f, 0x10
};
//u8g2.drawBitmap( 0, 0, 128, 64, myBitmap);
u8g2.drawXBMP( 0, 0, 128, 64, myBitmap);
//u8g2.sendBuffer(); // transfer internal memory to the display

*/
  u8g2.setFont(u8g2_font_courB24_tf);
  u8g2.setCursor(20, 43);
  u8g2.print("Moin!");
  
  u8g2.sendBuffer();
  // enable internal pull downs for digital inputs 
  pinMode(button_start_pin, INPUT_PULLDOWN);
  pinMode(button_stop_pin, INPUT_PULLDOWN);
  pinMode(switch_betrieb_pin, INPUT_PULLDOWN);
  pinMode(switch_setup_pin, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);

  // Rotary
  pinMode(outputA,INPUT);
  pinMode(outputB,INPUT);
  aLastState = digitalRead(outputA);
  //attachInterrupt(outputA, rotaryCheck, CHANGE);
  pinMode(outputSW, INPUT_PULLUP);
  //attachInterrupt(outputSW, isr1, FALLING);
  attachInterrupt(outputA, isr2, CHANGE);
 
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
  
  Serial.begin(115200);
  while (!Serial) {
  }
  
  //u8g2.begin();
  scale.begin(hx711_dt_pin, hx711_sck_pin);
  scale.power_up();

  servo.attach(servo_pin, 750, 2500);

  getPreferences();
  delay(4000);
}


void loop()
{
  if ((digitalRead(switch_setup_pin)) == HIGH) {
    Betriebsmodus = Setup;
    processSetup();
  }
  // Betrieb 
  if ((digitalRead(switch_betrieb_pin)) == HIGH) {
    Betriebsmodus = Betrieb;
    processBetrieb();
  }
  // Handbetrieb 
  if ((digitalRead(switch_betrieb_pin) == LOW) && (digitalRead(switch_setup_pin) == LOW)) {
    Betriebsmodus = Handbetrieb;
    processHandbetrieb();
  }
}

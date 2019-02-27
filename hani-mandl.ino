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
                            Anpassung fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
                            - pins bei OLED-Initialisierung geaendert
                            - pins geaendert, um Konflikte mit hard wired pins des OLEDs zu vermeiden 
  2019-02 Clemens Gruber  | Aktivierung der internen pull downs für alle digitalen Eingaenge
  2019-02 Clemens Gruber  | "normale" pins zu Vcc / GND geaendert um die Verkabelung etwas einfacher und angenehmer zu machen
 
                            
  This code is in the public domain.
  
  
  Hinweise zur Hardware
  ---------------------
  - bei allen digitalen Eingängen sind interne pull downs aktiviert, keine externen Widerstände nötig! 
*/

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>  /* aus dem BSP von expressif */
#include <HX711.h>        /* https://github.com/bogde/HX711 */
#include <ESP32_Servo.h>  /* https://github.com/jkb-git/ESP32Servo */
#include <AceButton.h>
#include <U8g2lib.h>      /* aus dem Bibliotheksverwalter */

using namespace ace_button;

// if you need debug output on the serial port, enable this
#define isDebug 

// if you need your display rotated, enable this
//#define DISPLAY_ROTATE

// if you need to flip the poti direction, enable this
#define FLIP_POTI

#ifdef isDebug
#define DebugOut(a) Serial.println(a)
#else
#define DebugOut(a) 
#endif

Servo servo;
HX711 scale;
Preferences preferences;

// ** Definition der pins 
// ----------------------

// OLED
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
// fuer Heltec WiFi Kit 32 (ESP32 onboard OLED) 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// Servo
const int servo_pin = 17;

// 3x Schalter Ein 1 - Aus - Ein 2
const int switch_betrieb_pin = 19;
const int switch_setup_pin = 21;

// Taster 
const int button_start_pin = 12;
const int button_stop_pin = 27;

// Poti
const int poti_pin = 39;

// Wägezelle-IC 
const int hx711_sck_pin = 22;
const int hx711_dt_pin = 13;

// wir verwenden auch den pin fuer LED_BUILTIN!
// es ist pin 25 fuer den Heltec WiFi Kit 32 

ButtonConfig buttonConfig;
AceButton bt_start(&buttonConfig);
AceButton bt_stop(&buttonConfig);

/*
// currently not used, as fire events after reboot is not supported
ButtonConfig switchConfig;
AceButton sw_betrieb(&switchConfig);
AceButton sw_setup(&switchConfig);
*/

enum menu_state_t { state_setup = 0, state_hand, state_betrieb };

menu_state_t main_state;

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
int winkel_max = 90;
int winkel_dosier_min = 60; //45
float fein_dosier_gewicht = 30; //60
int i;
int u;
int a;
int z;

void print2serial(String displayname, int value) {
#ifdef isDebug  
  Serial.print(displayname);
  Serial.println(value);
#endif  
}

void print2serial(String displayname, float value) {
#ifdef isDebug  
  Serial.print(displayname);
  Serial.println(value);
#endif
}

int read_poti(int map_min, int map_max)
{
#ifdef FLIP_POTI  
      return map(analogRead(poti_pin), 0, 4095, map_max, map_min);
#else
      return map(analogRead(poti_pin), 0, 4095, map_min, map_max);
#endif      
}

int read_scale(void)
{
  
}


void getPreferences(void) {
  // EEPROM //
  DebugOut(__FUNCTION__);

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
  tara = preferences.getUInt("tara", 0);
  tara_raw = preferences.getUInt("tara_raw", 0);
  fmenge = preferences.getUInt("fmenge", 0);
  korrektur = preferences.getUInt("korrektur", 0);
  autostart = preferences.getUInt("autostart", 0);

  print2serial("faktor = ", faktor);
  print2serial("tara_raw = ", tara_raw);
  print2serial("tara = ", tara);
  preferences.end();
}

void putPreferences(void) {
  preferences.begin("EEPROM", false);  
  preferences.putUInt("faktor2", (faktor * 10000));
  preferences.putUInt("tara", tara);
  preferences.putUInt("tara_raw", tara_raw);
  preferences.putUInt("fmenge", fmenge);
  preferences.putUInt("korrektur", korrektur);
  preferences.putUInt("autostart", autostart);
  preferences.end();
}

void setupTara(void) {
  DebugOut(__FUNCTION__);
  u8g2.setCursor(0, 8);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    tara = ((int(scale.read_average(10)) - tara_raw) / faktor);
    u8g2.setCursor(100, 12);
    u8g2.print("OK");
    u8g2.sendBuffer();
    delay(2000);
    putPreferences();
  }
}

void setupCalibration(void) {
  DebugOut(__FUNCTION__);
  u8g2.setCursor(0, 22);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
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
    
    while (i > 0) {
      if ((digitalRead(button_start_pin)) == HIGH) {
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
    u8g2.print("& mit Start");
    u8g2.setCursor(10, 60);
    u8g2.print("bestaetigen");
    u8g2.sendBuffer();
    
    while (i > 0) {
      if ((digitalRead(button_start_pin)) == HIGH) {
        tara_raw = (int(scale.read_average(10)));
        delay(2000);
        i = 0;
        faktor = ((gewicht_raw - tara_raw) / 500.000);

        putPreferences();
      }
    }
  }
}

void setupKorrektur(void) {
  DebugOut(__FUNCTION__);
  u8g2.setCursor(0, 36);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(300);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = read_poti(-50, 10);
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
      putPreferences();      
    }
  }
}

void setupFuellmenge(void) {
  DebugOut(__FUNCTION__);
  u8g2.setCursor(0, 50);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(200);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = read_poti(1, 4);

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
          putPreferences();      

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
          
          putPreferences();
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
          putPreferences();
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
          putPreferences();
        }
      }
    }
  }
}

void setupAutostart(void) {
  DebugOut(__FUNCTION__);
  u8g2.setCursor(0, 64);
  u8g2.print("*");
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    i = 1;
    delay(200);
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.clearBuffer();
    
    while (i > 0) {
      pos = read_poti(1, 2);

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
          
          putPreferences();
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
          putPreferences();
        }
      }
    }
  }
}

void processSetup(void) {
  DebugOut(__FUNCTION__);
  pos = read_poti(1, 5);


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
  a = 0;
}

void showMessage(String msg)
{
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_courB24_tf);
      u8g2.setCursor(20, 43);
      u8g2.print(msg);
      u8g2.sendBuffer();
}

void processBetrieb(void)
{
  DebugOut(__FUNCTION__);
  pos = read_poti(0, 100);

  gewicht = ((((int(scale.read())) - tara_raw) / faktor) - tara);
  
  if ((autostart == 1) && (gewicht <= 5) && (gewicht >= -5) && (a == 0)) {
      delay(1000);
      showMessage("START");
      delay(1500);
      a = 1;
    
  }
  
  if ((autostart == 1) && (gewicht < -20)) {
    winkel = winkel_min;
    a = 0;
  }
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    a = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    winkel = winkel_min;
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
  DebugOut("Servo Winkel:");
  DebugOut(winkel);

  float y = ((fmenge + korrektur - gewicht) / fein_dosier_gewicht);

  
  #ifdef isDebug
    Serial.println(y);    
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
  
  u8g2.setCursor(0, 64);
  u8g2.print("k=");
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
  DebugOut(__FUNCTION__);
  pos = read_poti(0, 100);

  gewicht = ((((int(scale.read())) - tara_raw) / faktor) - tara);
  
  if ((digitalRead(button_start_pin)) == HIGH) {
    a = 1;
  }
  
  if ((digitalRead(button_stop_pin)) == HIGH) {
    winkel = winkel_min;
    a = 0;
  }
  
  if (a == 1) {
    winkel = ((winkel_max * pos) / 100);
  }
  
  DebugOut("Servo Winkel:");
  DebugOut(winkel);
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
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t  buttonState ) {
  DebugOut("Handle Event");
  DebugOut("Button-ID:");
  DebugOut(button->getId());
  DebugOut("Button-eventtype:");
  DebugOut(eventType);
  DebugOut("Button-state:");
  DebugOut(buttonState);

  switch (eventType) {
    case AceButton::kEventReleased:
      // We trigger on the Released event not the Pressed event to distinguish
      // this event from the LongPressed event.
      //retrievePreset(button->getId());
      break;
    case AceButton::kEventLongPressed:
      //setPreset(button->getId());
      break;
  }
}

void setup()
{
  DebugOut(__FUNCTION__);
  // enable internal pull downs for digital inputs 
  pinMode(button_start_pin, INPUT_PULLDOWN);
  pinMode(button_stop_pin, INPUT_PULLDOWN);
  pinMode(switch_betrieb_pin, INPUT_PULLDOWN);
  pinMode(switch_setup_pin, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);

  // Configure the Button/Switch Configs with the event handler, and enable all higher
  // level events.
  bt_start.init(button_start_pin, LOW, 0);
  bt_stop.init(button_stop_pin, LOW, 1);
  /*
  sw_betrieb.init(switch_betrieb_pin, LOW, 2);
  sw_setup.init(switch_setup_pin, LOW, 3);
  */
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  /*
  switchConfig.setEventHandler(handleEvent);
  switchConfig.setFeature(ButtonConfig::kFeatureClick);
  switchConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  switchConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  */
  // switch Vcc / GND on normal pins for convenient wiring
  // output 5V for VCC
  /*
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
  */
#ifdef isDebug  
  Serial.begin(115200);
  while (!Serial) {
  }
#endif  
  u8g2.begin();

#ifdef DISPLAY_ROTATE
  u8g2.setDisplayRotation(U8G2_R0);
  u8g2.setFlipMode(1);   
#endif  
  scale.begin(hx711_dt_pin, hx711_sck_pin);
  scale.power_up();

  DebugOut("Servo ATTACH");

  servo.attach(servo_pin, 750, 2500);

  getPreferences();

}


void loop()
{
  bt_start.check();
  bt_stop.check();

  if ((digitalRead(switch_setup_pin)) == HIGH)
    processSetup();
  else if ((digitalRead(switch_betrieb_pin)) == HIGH)
    processBetrieb();
  else
    processHandbetrieb();

}

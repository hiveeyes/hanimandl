/*
  HaniMandl Version 0.3.0
  ------------------------
  Copyright (C) 2018-2023 by Marc Vasterling, Marc Wetzel, Clemens Gruber, Marc Junker, Andreas Holzhammer, Johannes Kuder, Jeremias Bruker

  2018-05 Marc Vasterling    | initial version,
                               publicized in the Facebook group "Imkerei und Technik. Eigenbau",
                               Marc Vasterling: "my code can be used, changed and uploaded, by anyone as long as they dont stick their name on it."
  2018-06 Marc Vasterling    | version upgrade,
                               publicized in the Facebook group
  2019-01 Marc Wetzel        | rebuilding and documentation,
                               publicized in the Facebook group
  2019-02 Clemens Gruber     | code beautifying with small renaming of variables and functions
                               adaptation for Heltec WiFi Kit 32 (ESP32 onboard OLED)
                               - pins changed at OLED intialization
                               - pins changed to avoid conflicts with hard wired OLED pins
  2019-02 Clemens Gruber     | activation of all internal pull-downs for all digital entries
  2019-02 Clemens Gruber     | "normal" pins to Vcc / GND changed to make cabling somewhat simpler and more agreable
  2020-05 Andreas Holzhammer | adaptations to the changed pin layout of Heltec version 2 ;-(
                               now sells as "New Wifi Kit 32" or "Wifi Kit 32 V2"
  2020-05 Marc Junker        | - extension from potentiometer to rotary encoder
                               - all Serial.prints in #ifdef locked
                               - "jar" now as array with individual tare
                               - correction value and choice of fillquantity now settable by clicking and rolling rotary encoder
  2020-05 Andreas Holzhammer | - tare for jar to be filled automatically adapted (variable tare_jar)
                               - code also works with no scale
  2020-06 Andreas Holzhammer | - code option to use Heltec V1 or V2
                               - code option to use potentiometer or rotatry encoder
                               - tare for jar configurable
                               - opening angle for maximum opening and fine metering can be configured in the setup
                               - correction and jar size can be selected in automatic mode using rotary encoder
                               - preferences resetable with setup menu
                               - weight flashes on full auto if jar not completely filled
                               - scale missing and scale not calibrated warnings added
                               - tare is only set >20g to prevent autostart when scale is empty
                               - scale zeroing at every startup to +-20g otherwise warning displayed
  2020-07 Andreas Holzhammer | Version 0.2.4
                               - SCALE_READS on set on 2? about 100ms faster than 3, but fluctuates around +-1g
                               - order of boot messages optimized so that only relevant warnings are issued
                               - implemented autocorrection
                               - LOGO! and credits (suggested by Johannes Kuder)
                               - stop button leaves setup undermenus (suggested by Johannes Kuder)
                               - preferences only saved when modified
  2020-07 Andreas Holzhammer | Version 0.2.5
                               - display of previous value in setup
                               - Overfill value in autocorrection settable
                               - setup cleaned up, minimum servo angle adjustable
  2020-07 Andreas Holzhammer | Version 0.2.6
                               - scale calibration upgrade; rounding up of measured values; scale "warm-up" before bootscreen
                               - active piezo buzzer(suggested by Johannes Kuder)
  2020-07 Johannes Kuder     | Version 0.2.7
                               - counter for filled jars and weight (only in auto mode)
  2020-07 Jeremias Bruker    | Version 0.2.8
                               - "JarType" integrated in every menus and automatic mode
                               - 5 different jars can be configured for weight and jar type by user in "Jar types" menu and are saved in a nonvolatile manner. This way every user can set their own 5 particular jars types
                               - stabilization of scale values at will (define ERRORCORRECTION_SCALE)
                               - calibration weight can be changed by user during calibration process
                                 (not everyone has 500g as a calibration weight) and is stored in a nonvolatile manner
                               - rotating main menu
                               - reversible servo for left-hand opening honeygate :-)
  2020-10 Andreas Holzhammer | Version 0.2.8.1
                               - bugfix: servo could be moved below minimum in manual mode
                               - jar tolerance can be controlled via variable and adjusted to +-20g
  2020-12 Andreas Holzhammer | Version 0.2.9.1
                               - progressbar built in
                               - adapted to ESP32Servo from the library manager
  2021-01 Andreas Motl       | Version 0.2.9.1
                               - PlatformIO support adapted to the new servo library
  2021-02 Andreas Holzhammer | Version 0.2.10
                               - correction can be set between -90 and +20
                               - autocorrection also without autostart
                               - flashing preferences confirmation implemented
  2021-07 Andreas Holzhammer | Version 0.2.11
                               - credits page
                               - fix for rotary with step > 1
  2021-11 Andreas Holzhammer | Version 0.2.12
                               - configurable jar tolerance
                               - confort adjustments for fill quantities (1g/5g/25g steps)
  2023-01 Clemens Gruber     | Version 0.2.13
                               - pin adaptation for hardware version V3 of Heltec "WiFi Kit 32 V3" with pin layout changed again
                               - default HARDWARE_LEVEL is now 3 / Heltec V3
                               - adaptation for ESP32 Arduino core version ≥ 2.x
                                - display, U8g2: HW instead of SW in constructor (possibly problems with olderHeltec versions)
                                - rotary: de-bouncing code in isr2 commented out, was leading to crashes
  2024-12 Jérémie Lehmann    | Version 0.3.0
                               -added structure for UI translation to multiple languages at compile time using resources_*.h files
                               -added support for Esp32_Devkitc_v4(38 pins) with external SPI OLED screen.
                               -translated all code and comments from the original German to English
                               -added English, German and French resources_*.h files
                               -added a second credits page for more contributors names

  This code is in the public domain.

  Notes on hardware
  ---------------------
  - Internal pull-downs are activated for all digital entries, no external resistances needed!

*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>      /* from the library manager */
#include <HX711.h>        /* from the library manager: "HX711 Arduino Library" by Bogdan Necula, Andreas Motl */
#include <ESP32Servo.h>   /* from the library manager */
#include <Preferences.h>  /* from the expressif BSP, available when the right board is chosen */

#define LANG_EN 0
#define LANG_DE 1
#define LANG_FR 2
#define LANGUAGE LANG_EN
#if LANGUAGE==LANG_EN
#include "resources_en.h"
#elif LANGUAGE==LANG_DE
#include "resources_de.h"
#elif LANGUAGE==LANG_FR
#include "resources_fr.h"
#else
#error "invalid language"
#endif

//
// Setup the code for the used hardware here
//
#define HARDWARE_LEVEL 4        // 1 = original layout with switch on pins 19/22/21
                                // 2 = layout for Heltec V2 switch on pins 23/19/22
                                // 3 = layout for Heltec V3 with completely changed pins
                                // 4 = layout for Esp32_Devkitc_v4(38 pins) with completely changed pins and SPI OLED
#define SERVO_EXTENDED       // define in case the hardware uses the old potentiometer or the servo moves to little
                                // if not, the servo stays open a few degrees in stop position! Check after updating!
#define ROTARY_SCALE 2          // in what steps the rotary encoder moves.
                                // Examples: KY-040 = 2, HW-040 = 1, for potentiometer use set to 1
#define USE_ROTARY              // wether to use rotary
#define USE_ROTARY_SW           // wether to use rotary switch
//#define USE_POTENTIOMETER     // Use potentiometer -> WARNING in normal usecase deactivate USE_ROTARY_SW!
//#define ERRORCORRECTION_SCALE // if weight jumps occur, they can be caught here
                                // Warning: this can slow down the weighing process. Check hardware beforehand.
//#define SERVO_INVERTED        // Inverses the servo, in case of a left-hand opening honeygate. Examples of this are known.
//
// End of user setup!
//

//
// From here on, only change if you know exactly what you're doing!
//
// #define isDebug 4            // Activate serial debug output. Values > 3 will output every measurement
                                // set to 4 for additional lead times
                                // set to 5 for additional rotary debug infos
                                // WARNING: to many serial outputs can trigger an ISR watchdog reset!!
//#define POTISCALE             // Potentiometer simulates a weightcell, for testing purposes only!
#define MAXIMUMWEIGHT 1000      // Maximum Weight

// Driving of scale
#define SCALE_READS 2      // Parameters for hx711 library. Measured value displayed on screen
#define SCALE_GETUNITS(n)  (scale_present ? round(scale.get_units(n)) : simulate_scale(n) )

// Driving of servo
#ifdef SERVO_INVERTED
#define SERVO_WRITE(n)     servo.write(180-n)
#else
#define SERVO_WRITE(n)     servo.write(n)
#endif

// Rotary encoder switch pulls level to LOW, Start/Stop to high!
#ifdef USE_ROTARY_SW
#define SELECT_SW outputSW
#define SELECT_LEVEL LOW
#else
#define SELECT_SW button_start_pin
#define SELECT_LEVEL HIGH
#endif

// Operation mode
#define MODE_SETUP       0
#define MODE_AUTOMATIC   1
#define MODE_MANUAL      2

// Buzzer sounds
#define BUZZER_SHORT   1
#define BUZZER_LONG    2
#define BUZZER_SUCCESS 3
#define BUZZER_ERROR   4

// ** Pins definitions
// ----------------------

//Esp32_Devkitc_V4

#if HARDWARE_LEVEL == 4
// external OLED for Esp32_Devkitc_v4(38 pins)
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// for the ESP32 Arduino core Version ≥ 2.x we need HW I2C, runs to slow with SW I2C
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 21, /* clock=*/ 18, /* data=*/ 17);
U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 19, /* data=*/ 18, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16); // SPI pinout without soldering JL


// Rotary encoder
const int outputA  = 34;  // Clk
const int outputB  = 35;  // DT
const int outputSW = 32;

// Servo
const int servo_pin = 2;

//  3-way selector ON 1 - OFF - ON 2
const int switch_mode_pin    = 26;
const int switch_vcc_pin     = 25;     // <- Vcc
const int switch_setup_pin   = 33;
const int vext_ctrl_pin      = 0;      // Vext control pin

// Buttons
const int button_start_vcc_pin =  27;  // <- Vcc
const int button_start_pin     =  14;
const int button_stop_vcc_pin  =  12;  // <- Vcc
const int button_stop_pin      =  13;

// Potentiometer
//const int poti_pin = 39;

// Weightcell-IC
const int hx711_sck_pin = 23;
const int hx711_dt_pin  = 22;

// Buzzer - active piezo
static int buzzer_pin = 21;



// Heltec Version 3
//
#elif HARDWARE_LEVEL == 3
// OLED for Heltec WiFi Kit 32 (ESP32 onboard OLED)
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// for the ESP32 Arduino core Version ≥ 2.x we need HW I2C, runs to slow with SW I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 21, /* clock=*/ 18, /* data=*/ 17);

// Rotary encoder
const int outputA  = 47;  // Clk
const int outputB  = 48;  // DT
const int outputSW = 26;

// Servo
const int servo_pin = 33;

// 3-way selector ON 1 - OFF - ON 2
const int switch_mode_pin    = 40;
const int switch_vcc_pin     = 41;     // <- Vcc
const int switch_setup_pin   = 42;
const int vext_ctrl_pin      = 36;     // Vext control pin

// Buttons

const int button_start_vcc_pin =  7;  // <- Vcc
const int button_start_pin     =  6;
const int button_stop_vcc_pin  =  5;  // <- Vcc
const int button_stop_pin      =  4;

// Potentiometer
//const int poti_pin = 39;

// Weightcell-IC
const int hx711_sck_pin = 38;
const int hx711_dt_pin  = 39;

// Buzzer - active piezo
static int buzzer_pin = 2;


#elif HARDWARE_LEVEL == 2
//
// Heltec Version 2

// OLED for Heltec WiFi Kit 32 (ESP32 onboard OLED)
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// for the ESP32 Arduino core Version ≥ 2.x we need HW I2C, runs to slow with SW I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);

// Rotary encoder
const int outputA  = 33;
const int outputB  = 26;
const int outputSW = 32;

// Servo
const int servo_pin = 2;

// 3-way selector ON 1 - OFF - ON 2
const int switch_mode_pin    = 23;
const int switch_vcc_pin     = 19;     // <- Vcc
const int switch_setup_pin   = 22;
const int vext_ctrl_pin      = 21;     // Vext control pin

// Buttons
const int button_start_vcc_pin = 13;   // <- Vcc
const int button_start_pin     = 12;
const int button_stop_vcc_pin  = 14;   // <- Vcc
const int button_stop_pin      = 27;

// Potentiometer
const int poti_pin = 39;

// Weightcell-IC
const int hx711_sck_pin = 17;
const int hx711_dt_pin  = 5;

// Buzzer - active piezo
static int buzzer_pin = 25;


#elif HARDWARE_LEVEL == 1
//
// Heltec Version 1

// OLED for Heltec WiFi Kit 32 (ESP32 onboard OLED)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);   // HW I2C crashed the code

// Rotary encoder
const int outputA  = 33;
const int outputB  = 26;
const int outputSW = 32;

// Servo
const int servo_pin = 2;

// 3-way selector ON 1 - OFF - ON 2
const int switch_mode_pin    = 19;
const int switch_vcc_pin     = 22;     // <- Vcc
const int switch_setup_pin   = 21;

// Buttons
const int button_start_vcc_pin = 13;   // <- Vcc
const int button_start_pin     = 12;
const int button_stop_vcc_pin  = 14;   // <- Vcc
const int button_stop_pin      = 27;

// Potentiometer
const int poti_pin = 39;

// Weightcell-IC
const int hx711_sck_pin = 17;
const int hx711_dt_pin  = 5;

// Buzzer - active piezo
static int buzzer_pin = 25;
#else
#error Hardware level not defined! Set a valid #define
#endif


Servo servo;
HX711 scale;
Preferences preferences;

// Data structure for Rotary Encoder
struct rotary {
  int Value;
  int Minimum;
  int Maximum;
  int Step;
};
#define SW_ANGLE     0
#define SW_CORRECTION 1
#define SW_MENU      2
struct rotary rotaries[3];         // Initialized in setup()
int rotary_select = SW_ANGLE;
static boolean rotating = false;   // debounce management for rotary encoder

// Fill quantities for 5 different jars
struct jar {
  int Weight;
  int JarType;    //JB
  int Tare;
  int TripCount;  //Kud
  int Count;      //Kud
};
const char *JarTypeArray[3] = { JARTYPE_1, JARTYPE_2, JARTYPE_3};//DIB = DeutscherImkerBund jars, DEE= DeepTwist jars, TOF or TO= TwistOff jars //JB, JL
struct jar jars[5] =            {
                                         {  125, 0, -9999, 0, 0 },
                                         {  250, 1, -9999, 0, 0 },
                                         {  250, 2, -9999, 0, 0 },
                                         {  500, 1, -9999, 0, 0 },
                                         {  500, 0, -9999, 0, 0 }
                                    };

// General variables
int i;                         // general count variable
int pos;                       // current position of potentiometer or rotary
int weight;                    // current weight
int tare;                      // tare for chosen jar, for automatic mode
int tare_jar;                  // tare for current jar, in case jar weight differs
long weight_empty;             // weight of empty scale
float factor;                  // scaling factor for scale values
int fquantity;                 // chosen fill quantity
int fquantity_index;           // index in jars[]
int correction;                // correction value for fill quantity
int autostart;                 // full automatic on/off
int autocorrection;            // autocorrection on/off
int overfill_gr;               // desired overfilling for autcorrection mode in grams
int angle;                     // current servo angle
int angle_hard_min = 0;        // hard limit for servo
int angle_hard_max = 180;      // hard limit for servo
int angle_min = 0;             // configurable in setup
int angle_max = 85;            // configurable in setup
int angle_fine = 35;           // configurable in setup
float fine_dosage_weight = 60; // float due to calculation of the closing angle
int servo_enabled = 0;         // activate servo yes/no
int cali_weight = 500;         // choosen weight for calibration
char output[30];               // Fontsize 12 = 13 maximum characters per line
int mode = -1;                 // wether to drive the servo to minimum on mode change
int auto_enabled = 0;          // for automatic mode system on/off?
int scale_present = 0;         // not talking to HX711 when no scale is connected, otherwise crash
long preferences_chksum;       // checksum to not write uncoherent prefs
int buzzermode = 0;            // 0 = off, 1 = on. TODO: button sounds as buzzermode 2?
bool counted = true;           // Kud count flag
bool setup_modern = 1;         // Setup appearance as rolling menu
int jartolerance = 20;         // weight for autostart may vary by +-20g, total 40g!

// Simulates the duration of the weighing process when no scale is connected. Affects the blinking frequency in automatic mode.
long simulate_scale(int n) {
    long sim_weight = 9500;
    while (n-- >= 1) {
      delay(10);    // empirically determined. n=2: 10, n=3: 40, n=4: 50
    }
#ifdef POTISCALE
    sim_weight = (map(analogRead(poti_pin), 0, 4095, 0, 700));
#endif
    return sim_weight;
}

#ifdef USE_ROTARY_SW
// Rotary button. The interrupt only takes effect in automatic mode and only when servo is inactive.
// The button switches to one of three modes in which different values ​​are counted.
void IRAM_ATTR isr1() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 300) {        // If interrupts come faster than 300ms, assume it's a bounce and ignore
    if ( mode == MODE_AUTOMATIC && servo_enabled == 0 ) {  // the click is only relevant in automatic mode
      rotary_select = (rotary_select + 1) % 3;
#ifdef isDebug
    Serial.print("Rotary button changed to ");
    Serial.println(rotary_select);
#endif
    }
    last_interrupt_time = interrupt_time;
  }
}
#endif

#ifdef USE_ROTARY
// Rotary encoder. Counts into one of three data structures:
// SW_ANGLE    = Setup of the servo angle
// SW_CORRECTION = Correction factor for fill weight
// SW_MENU      = Counter for menu selections
void IRAM_ATTR isr2() {
  static int aState;
  static int aLastState = 2;  // real values are 0 and 1

// Commented out since the delay in isr function probably triggers a reset from Version ≥ 2.x of the ESP32 Arduino core
// Observed no negative effect of bouncing during rudimentary tests (cg)
//  if ( rotating ) delay (1);  // wait a little until the bouncing is done

  aState = digitalRead(outputA); // Reads the "current" state of the outputA
    if (aState != aLastState) {
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      if (digitalRead(outputB) != aState) {
         rotaries[rotary_select].Value -= rotaries[rotary_select].Step;
      } else {    // counter-clockwise
         rotaries[rotary_select].Value += rotaries[rotary_select].Step;
      }
      rotaries[rotary_select].Value = constrain( rotaries[rotary_select].Value, rotaries[rotary_select].Minimum, rotaries[rotary_select].Maximum );
      rotating = false;
#ifdef isDebug
#if isDebug >= 5
      Serial.print(" Rotary value changed to ");
      Serial.println(getRotariesValue(rotary_select));
#endif
#endif
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
}
#endif

//
// Scaling of rotoray for different rotary encoders or simulation via potentiometer
int getRotariesValue( int rotary_mode ) {
#ifdef USE_ROTARY
    return ( (rotaries[rotary_mode].Value - (rotaries[rotary_mode].Value % (rotaries[rotary_mode].Step*ROTARY_SCALE) )) / ROTARY_SCALE );
#elif defined USE_POTENTIOMETER
    int poti_min = (rotaries[rotary_mode].Minimum / ROTARY_SCALE);
    int poti_max = (rotaries[rotary_mode].Maximum / ROTARY_SCALE);
    if( rotaries[rotary_mode].Step > 0 ) {
       return (map(analogRead(poti_pin), 0, 4095, poti_min, poti_max));
    } else {
       return (map(analogRead(poti_pin), 0, 4095, poti_max, poti_min));
    }
#else
#error Neither rotary nor potentiometer activated!
#endif
}
void setRotariesValue( int rotary_mode, int rotary_value ) {
    rotaries[rotary_mode].Value = rotary_value * ROTARY_SCALE;
}
void initRotaries( int rotary_mode, int rotary_value, int rotary_min, int rotary_max, int rotary_step ) {
    rotaries[rotary_mode].Value     = rotary_value * ROTARY_SCALE;
    rotaries[rotary_mode].Minimum   = rotary_min   * ROTARY_SCALE;
    rotaries[rotary_mode].Maximum   = rotary_max   * ROTARY_SCALE;
    rotaries[rotary_mode].Step      = rotary_step;

#ifdef isDebug
    Serial.print("initRotaries...");
    Serial.print(" Rotary Mode: ");  Serial.print(rotary_mode);
    Serial.print(" rotary_value: "); Serial.print(rotary_value);
    Serial.print(" Value: ");        Serial.print(rotaries[rotary_mode].Value);
    Serial.print(" Min: ");          Serial.print(rotaries[rotary_mode].Minimum);
    Serial.print(" Max: ");          Serial.print(rotaries[rotary_mode].Maximum);
    Serial.print(" Step: ");         Serial.print(rotaries[rotary_mode].Step);
    Serial.print(" Scale: ");        Serial.println(ROTARY_SCALE);
#endif
}
// End functions for rotary encoder
//


void getPreferences(void) {
    preferences.begin("EEPROM", false);            // Read parameters from EEPROM

    factor        = preferences.getFloat("factor", 0.0);  // in case this is not set -> scale is not calibrated
    pos           = preferences.getUInt("pos", 0);
    weight_empty  = preferences.getUInt("weight_empty", 0);
    correction     = preferences.getUInt("correction", 0);
    autostart     = preferences.getUInt("autostart", 0);
    autocorrection = preferences.getUInt("autocorrection", 0);
    overfill_gr     = preferences.getUInt("overfill_gr", 5);
    fquantity_index  = preferences.getUInt("fquantity_index", 3);
    angle_min    = preferences.getUInt("angle_min", angle_min);
    angle_max    = preferences.getUInt("angle_max", angle_max);
    angle_fine   = preferences.getUInt("angle_fine", angle_fine);
    buzzermode    = preferences.getUInt("buzzermode", buzzermode);
    cali_weight  = preferences.getUInt("cali_weight", cali_weight); //JB
    setup_modern  = preferences.getUInt("setup_modern", setup_modern);
    jartolerance   = preferences.getUInt("jartolerance", jartolerance);

    preferences_chksum = factor + pos + weight_empty + correction + autostart + autocorrection + fquantity_index + angle_min + angle_max + angle_fine + overfill_gr + buzzermode + cali_weight + setup_modern + jartolerance;

    i = 0;
    int ResetWeights[] = {125,250,250,500,500,};
    int ResetJarTypes[] = {0,1,2,1,0,};
    while( i < 5) {
      sprintf(output, "Weight%d", i); //JB
      jars[i].Weight = preferences.getInt(output, ResetWeights[i]); //JB
      preferences_chksum += jars[i].Weight; //JB

      sprintf(output, "JarType%d", i); //JB
      jars[i].JarType = preferences.getInt(output, ResetJarTypes[i]); //JB
      preferences_chksum += jars[i].JarType; //JB

      sprintf(output, "Tare%d", i);
      jars[i].Tare= preferences.getInt(output, -9999);
      preferences_chksum += jars[i].Tare;

      sprintf(output, "TripCount%d", i); //Kud
      jars[i].TripCount = preferences.getInt(output, 0);//Kud
      preferences_chksum += jars[i].TripCount;

      sprintf(output, "Count%d", i); //Kud
      jars[i].Count = preferences.getInt(output, 0);//Kud
      preferences_chksum += jars[i].Count;
      i++;
    }

    preferences.end();

#ifdef isDebug
    Serial.println("get Preferences:");
    Serial.print("pos = ");          Serial.println(pos);
    Serial.print("factor = ");       Serial.println(factor);
    Serial.print("weight_empty = "); Serial.println(weight_empty);
    Serial.print("correction = ");    Serial.println(correction);
    Serial.print("autostart = ");    Serial.println(autostart);
    Serial.print("autocorrection = ");Serial.println(autocorrection);
    Serial.print("overfill_gr = ");    Serial.println(overfill_gr);
    Serial.print("fquantity_index = "); Serial.println(fquantity_index);
    Serial.print("angle_min = ");   Serial.println(angle_min);
    Serial.print("angle_max = ");   Serial.println(angle_max);
    Serial.print("angle_fine = ");  Serial.println(angle_fine);
    Serial.print("buzzermode = ");   Serial.println(buzzermode);
    Serial.print("cali_weight = "); Serial.println(cali_weight);//JB
    Serial.print("setup_modern = "); Serial.println(setup_modern);
    Serial.print("jartolerance = "); Serial.println(jartolerance);

    i = 0;
    while( i < 5 ) {
      sprintf(output, "Weight%d = ", i);
      Serial.print(output);
      Serial.println(jars[i].Weight);

      sprintf(output, "JarType%d = ", i);
      Serial.print(output);
      Serial.println(JarTypeArray[jars[i].JarType]);

      sprintf(output, "Tare%d = ", i);
      Serial.print(output);
      Serial.println(jars[i].Tare);

      i++;
    }
    Serial.print("Checksum:");
    Serial.println(preferences_chksum);
#endif
}

void setPreferences(void) {
  long preferences_newchksum;
  int angle = getRotariesValue(SW_ANGLE);
  int i;

  preferences.begin("EEPROM", false);

  // Angle setup dealt with separately, changes often
  if ( angle != preferences.getUInt("pos", 0) ) {
    preferences.putUInt("pos", angle);
#ifdef isDebug
    Serial.print("angle saved: ");
    Serial.println(angle);
#endif
  }

  // Counter dealt with separately, changes often
  for ( i=0 ; i < 5; i++ ) {
    sprintf(output, "TripCount%d", i);
    if ( jars[i].TripCount != preferences.getInt(output, 0) )
      preferences.putInt(output, jars[i].TripCount);
    sprintf(output, "Count%d", i);
    if ( jars[i].Count != preferences.getInt(output, 0) )
      preferences.putInt(output, jars[i].Count);
#ifdef isDebug
      Serial.print("Counter saved: Index ");
      Serial.print(i);
      Serial.print(" Trip ");
      Serial.print(jars[fquantity_index].TripCount);
      Serial.print(" Total ");
      Serial.println(jars[fquantity_index].Count);
#endif
  }

  // We're doing everything else together, it is rather static
  preferences_newchksum = factor + weight_empty + correction + autostart + autocorrection +
                          fquantity_index + angle_min + angle_max + angle_fine + overfill_gr +
                          buzzermode + cali_weight + setup_modern + jartolerance;
  i = 0;
  while( i < 5 ) {
    preferences_newchksum += jars[i].Weight;//JB
    preferences_newchksum += jars[i].JarType;//JB
    preferences_newchksum += jars[i].Tare;
//    preferences_newchksum += jars[i].TripCount;//Kud
//    preferences_newchksum += jars[i].Count;//Kud
    i++;
  }

  if( preferences_newchksum == preferences_chksum ) {
#ifdef isDebug
//    Serial.println("Preferences unchanged");
#endif
    return;
  }
  preferences_chksum = preferences_newchksum;

//    preferences.begin("EEPROM", false);
    preferences.putFloat("factor", factor);
    preferences.putUInt("weight_empty", weight_empty);
//    preferences.putUInt("pos", angle);
    preferences.putUInt("correction", correction);
    preferences.putUInt("autostart", autostart);
    preferences.putUInt("autocorrection", autocorrection);
    preferences.putUInt("overfill_gr", overfill_gr);
    preferences.putUInt("angle_min", angle_min);
    preferences.putUInt("angle_max", angle_max);
    preferences.putUInt("angle_fine", angle_fine);
    preferences.putUInt("fquantity_index", fquantity_index);
    preferences.putUInt("buzzermode", buzzermode);
    preferences.putUInt("cali_weight", cali_weight);//JB
    preferences.putUInt("setup_modern", setup_modern);
    preferences.putUInt("jartolerance", jartolerance);

    i = 0;
    while( i < 5 ) {
      sprintf(output, "Weight%d", i);
      preferences.putInt(output, jars[i].Weight);
      sprintf(output, "JarType%d", i);
      preferences.putInt(output, jars[i].JarType);
      sprintf(output, "Tare%d", i);
      preferences.putInt(output, jars[i].Tare);
//      sprintf(output, "TripCount%d", i);
//      preferences.putInt(output, jars[i].TripCount);//Kud
//      sprintf(output, "Count%d", i);
//      preferences.putInt(output, jars[i].Count);//Kud
      i++;
    }
    preferences.end();

#ifdef isDebug
    Serial.println("Set Preferences:");
    Serial.print("pos = ");          Serial.println(angle);
    Serial.print("factor = ");       Serial.println(factor);
    Serial.print("weight_empty = "); Serial.println(weight_empty);
    Serial.print("correction = ");    Serial.println(correction);
    Serial.print("autostart = ");    Serial.println(autostart);
    Serial.print("autocorrection = ");Serial.println(autocorrection);
    Serial.print("overfill_gr = ");    Serial.println(overfill_gr);
    Serial.print("fquantity_index = "); Serial.println(fquantity_index);
    Serial.print("angle_min = ");   Serial.println(angle_min);
    Serial.print("angle_max = ");   Serial.println(angle_max);
    Serial.print("angle_fine = ");  Serial.println(angle_fine);
    Serial.print("buzzermode = ");   Serial.println(buzzermode);
    Serial.print("cali_weight = "); Serial.println(cali_weight); //JB
    Serial.print("setup_modern = "); Serial.println(setup_modern);
    Serial.print("jartolerance = "); Serial.println(jartolerance);

    i = 0;
    while( i < 5 ) {
      sprintf(output, "Weight%d = ", i);
      Serial.print(output);         Serial.println(jars[i].Weight);
      sprintf(output, "JarType%d = ", i);
      Serial.print(output);         Serial.println(JarTypeArray[jars[i].JarType]);
      sprintf(output, "Tare%d = ", i);
      Serial.print(output);         Serial.println(jars[i].Tare);
      i++;
    }
#endif
}

void setupTripCounter(void) { //Kud
  int j;
  i = 1;
  float TripFillingweight = 0;

  while (i > 0) { //First screen: count per jar size
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    u8g2.setFont(u8g2_font_courB08_tf);
    u8g2.clearBuffer();
    while ( j < 5 ) {
      u8g2.setCursor(1, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY1, jars[j].Weight, JarTypeArray[jars[j].JarType]);
      u8g2.print(output);
      u8g2.setCursor(60, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY2, jars[j].TripCount);
      u8g2.print(output);
      j++;
    }
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Second screen: weight per jar size
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    u8g2.setFont(u8g2_font_courB08_tf);
    u8g2.clearBuffer();
    while ( j < 5  ) {
      u8g2.setCursor(1, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY1, jars[j].Weight,JarTypeArray[jars[j].JarType]);
      u8g2.print(output);
      u8g2.setCursor(65, 10 + (j * 13));
      //      Serial.println(jars[j].Weight);
      //      Serial.print("\t");
      //      Serial.print(jars[j].TripCount);
      //      Serial.print("\t");
      //      Serial.print(jars[j].Weight * jars[j].TripCount / 1000.0);
      //      Serial.println();

      sprintf(output, "%5.1fkg", jars[j].Weight * jars[j].TripCount / 1000.0);
      u8g2.print(output);
      j++;
    }
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Third screen: total weight
    TripFillingweight = 0;
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    while ( j < 5) {
      TripFillingweight += jars[j].Weight * jars[j].TripCount / 1000.0;
      j++;
    }
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB12_tf);
    u8g2.setCursor(5, 15);
    u8g2.print(COUNT_DISPLAY3);
    u8g2.setFont(u8g2_font_courB18_tf);
    u8g2.setCursor(10, 50);
    sprintf(output, "%5.1fkg", TripFillingweight);
    u8g2.print(output);
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Fourth screen: reset
    initRotaries(SW_MENU, 1, 0, 1, -1);

    i = 1;
    while (i > 0) {
      if ((digitalRead(button_stop_pin)) == HIGH)
        return;

      pos = getRotariesValue(SW_MENU);
      u8g2.setFont(u8g2_font_courB10_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);    u8g2.print(DIALOG_RESET);
      u8g2.setCursor(10, 28);    u8g2.print(DIALOG_CANCEL);

      u8g2.setCursor(0, 12 + ((pos) * 16));
      u8g2.print("*");
      u8g2.sendBuffer();

      if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
        u8g2.setCursor(105, 12 + ((pos) * 16));
        u8g2.print(DIALOG_OK);
        u8g2.sendBuffer();
        if ( pos == 0) {
          j = 0;
          while ( j < 5  ) {
            jars[j].TripCount = 0;
            j++;
          }
          setPreferences();
        }
        delay(1000);
        i = 0;
      }
    }
  }
}

void setupCounter(void) { //Kud
  int j;
  i = 1;
  float Fillingweight = 0;

  while (i > 0) { //First screen: count per jar size
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    u8g2.setFont(u8g2_font_courB08_tf);
    u8g2.clearBuffer();
    while ( j < 5 ) {
      u8g2.setCursor(1, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY1, jars[j].Weight,JarTypeArray[jars[j].JarType]);
      u8g2.print(output);
      u8g2.setCursor(60, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY2, jars[j].Count);
      u8g2.print(output);
      j++;
    }
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Second screen: weight per jar size
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    u8g2.setFont(u8g2_font_courB08_tf);
    u8g2.clearBuffer();
    while ( j < 5) {
      u8g2.setCursor(1, 10 + (j * 13));
      sprintf(output, COUNT_DISPLAY1, jars[j].Weight,JarTypeArray[jars[j].JarType]);
      u8g2.print(output);
      u8g2.setCursor(65, 10 + (j * 13));
      //      Serial.println(jars[j].Weight);
      //      Serial.print("\t");
      //      Serial.print(jars[j].Count);
      //      Serial.print("\t");
      //      Serial.print(jars[j].Weight * jars[j].Count / 1000.0);
      //      Serial.println();

      sprintf(output, "%5.1fkg", jars[j].Weight * jars[j].Count / 1000.0);
      u8g2.print(output);
      j++;
    }
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Third screen: total weight
    Fillingweight = 0;
    j = 0;
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      // Leave screen
      i = 0;
      delay(250);
    }

    while ( j < 5  ) {
      Fillingweight += jars[j].Weight * jars[j].Count / 1000.0;
      j++;
    }
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.setCursor(1, 15);
    u8g2.print(COUNT_DISPLAY4);
    u8g2.setFont(u8g2_font_courB18_tf);
    u8g2.setCursor(10, 60);
    sprintf(output, "%5.1fkg", Fillingweight);
    u8g2.print(output);
    u8g2.sendBuffer();
    delay(100);
  }

  i = 1;
  while (i > 0) { //Fourth screen: reset
    initRotaries(SW_MENU, 1, 0, 1, -1);

    i = 1;
    while (i > 0) {
      if ((digitalRead(button_stop_pin)) == HIGH)
        return;

      pos = getRotariesValue(SW_MENU);
      u8g2.setFont(u8g2_font_courB10_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);    u8g2.print(DIALOG_RESET);
      u8g2.setCursor(10, 28);    u8g2.print(DIALOG_CANCEL);

      u8g2.setCursor(0, 12 + ((pos) * 16));
      u8g2.print("*");
      u8g2.sendBuffer();

      if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
        u8g2.setCursor(105, 12 + ((pos) * 16));
        u8g2.print(DIALOG_OK);
        u8g2.sendBuffer();
        if ( pos == 0) {
          j = 0;
          while ( j < 5  ) {
            jars[j].Count = 0;
            jars[j].TripCount = 0;
            j++;
          }
          setPreferences();

        }
        delay(1000);
        i = 0;
      }
    }
  }
}


void setupTare(void) {
    int j;
    tare = 0;

    initRotaries( SW_MENU, fquantity_index, 0, 4, -1 );   // Set Encoder to Menu Mode, four Selections, inverted count

    i = 0;
    while ( i == 0 ) {
      if ((digitalRead(button_stop_pin)) == HIGH)
         return;

      if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) {
        tare = (int(SCALE_GETUNITS(10)));
        if ( tare > 20 ) {                  // Jars must be 20g minimum
           jars[getRotariesValue(SW_MENU)].Tare = tare;
        }
        i++;
      }

      u8g2.setFont(u8g2_font_courB10_tf);
      u8g2.clearBuffer();

      j = 0;
      while( j < 5  ) {
        u8g2.setCursor(3, 10+(j*13));
        if ( jars[j].Weight < 1000 ) {
          sprintf(output, " %3d-%3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
        } else {
          sprintf(output, " %3s-%3s", "1kg", JarTypeArray[jars[j].JarType]);
        }
        u8g2.print(output);
        u8g2.setCursor(75, 10+(j*13));
        if ( jars[j].Tare > 0 ) {
          sprintf(output, " %4dg", jars[j].Tare);
          u8g2.print(output);
        } else {
          u8g2.print(DIALOG_MISSING);
        }
        j++;
      }
      u8g2.setCursor(0, 10+(getRotariesValue(SW_MENU)*13) );
      u8g2.print("*");
      u8g2.sendBuffer();
    }
    delay(2000);
}


void setupCalibration(void) {
  float weight_raw;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB12_tf);
  u8g2.setCursor(0, 12);    u8g2.print(CALIBRATION_PROMPT1);
  u8g2.setCursor(0, 28);    u8g2.print(CALIBRATION_PROMPT2);
  u8g2.setCursor(0, 44);    u8g2.print(CALIBRATION_PROMPT3);
  u8g2.setCursor(0, 60);    u8g2.print(CALIBRATION_PROMPT4);
  u8g2.sendBuffer();

  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      scale.set_scale();
      scale.tare(10);
      delay(500);
        i = 0;
    }
  }

  u8g2.setFont(u8g2_font_courB12_tf);
  initRotaries( SW_MENU, cali_weight, 100, 9999, 1 );
  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    cali_weight = getRotariesValue(SW_MENU);

    int blinktime = (millis()/10) % 5;
    u8g2.clearBuffer();
    u8g2.setCursor(0, 12);u8g2.print(CALIBRATION_PROMPT5);

    if (blinktime < 3) {
      sprintf(output, "%dg", cali_weight);
    } else {
      sprintf(output, "     ");
    }
    u8g2.print(output);
    u8g2.setCursor(0, 28);    u8g2.print(CALIBRATION_PROMPT6);
    u8g2.setCursor(0, 44);    u8g2.print(CALIBRATION_PROMPT3);
    u8g2.setCursor(0, 60);    u8g2.print(CALIBRATION_PROMPT4);
    u8g2.sendBuffer();

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 12);u8g2.print(CALIBRATION_PROMPT5);
      sprintf(output, "%dg", cali_weight);
      u8g2.print(output);
      u8g2.setCursor(0, 28);    u8g2.print(CALIBRATION_PROMPT6);
      u8g2.setCursor(0, 44);    u8g2.print(CALIBRATION_PROMPT3);
      u8g2.setCursor(0, 60);    u8g2.print(CALIBRATION_PROMPT4);
      u8g2.sendBuffer();
      weight_raw  = scale.get_units(10);
      factor       = weight_raw / cali_weight;
      scale.set_scale(factor);
      weight_empty = scale.get_offset();    // Save the empty scale weight
#ifdef isDebug
      Serial.print("calibration_weight = ");
      Serial.println(cali_weight);
      Serial.print("weight_empty = ");
      Serial.println(weight_empty);
      Serial.print("weight_raw = ");
      Serial.println(weight_raw);
      Serial.print(" Factor = ");
      Serial.println(factor);
#endif
      delay(1000);
      i = 0;
    }
  }
}

void setupCorrection(void) {
    int correction_old = correction;

    rotary_select = SW_CORRECTION;

    i = 1;
    while (i > 0) {
      if ((digitalRead(button_stop_pin)) == HIGH) {
         setRotariesValue(SW_CORRECTION, correction_old);
         correction = correction_old;
         rotary_select = SW_MENU;
         return;
      }

      correction = getRotariesValue(SW_CORRECTION);
      u8g2.setFont(u8g2_font_courB12_tf);
      u8g2.clearBuffer();
      u8g2.setCursor(10, 12);
      u8g2.print(CORRECTION_CORR);
      u8g2.setCursor(40, 28);
      u8g2.print(correction);

      u8g2.setCursor(10, 48);     // A.P.
      u8g2.print(CORRECTION_PREVIOUS);   // A.P.
      u8g2.setCursor(40, 64);     // A.P.
      u8g2.print(correction_old);  // A.P.

      u8g2.sendBuffer();

      if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
        u8g2.setCursor(100, 28);
        u8g2.print(DIALOG_OK);
        u8g2.sendBuffer();
        delay(1000);
        i = 0;
      }
    }
    rotary_select = SW_MENU;
}

void setupServoWinkel(void) {
  int menuitem;
  int lastmin  = angle_min;
  int lastfine = angle_fine;
  int lastmax  = angle_max;
  int value_old;
  bool value_change = false;
  bool servo_live = false;

  initRotaries(SW_MENU, 0, 0, 4, -1);

  u8g2.setFont(u8g2_font_courB10_tf);
  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH) {
      angle_min  = lastmin;
      angle_fine = lastfine;
      angle_max  = lastmax;
      if ( servo_live == true ) SERVO_WRITE(angle_min);
      return;
    }

    if ( value_change == false ) {
      menuitem = getRotariesValue(SW_MENU);
    } else {
      switch (menuitem) {
        case 0: servo_live  = getRotariesValue(SW_MENU);
                break;
        case 1: angle_min  = getRotariesValue(SW_MENU);
                if ( servo_live == true ) SERVO_WRITE(angle_min);
                break;
        case 2: angle_fine = getRotariesValue(SW_MENU);
                if ( servo_live == true ) SERVO_WRITE(angle_fine);
                break;
        case 3: angle_max  = getRotariesValue(SW_MENU);
                if ( servo_live == true ) SERVO_WRITE(angle_max);
                break;
      }
    }

    u8g2.clearBuffer();
    u8g2.setCursor(10, 23); sprintf(output,SERVO_MINIMUM, angle_min);  u8g2.print(output);
    u8g2.setCursor(10, 36); sprintf(output,SERVO_FINE, angle_fine); u8g2.print(output);
    u8g2.setCursor(10, 49); sprintf(output,SERVO_MAXIMUM, angle_max);  u8g2.print(output);
    u8g2.setCursor(10, 62); u8g2.print(     DIALOG_SAVE);

    if ( value_change == false ) {
       u8g2.setCursor(10, 10); sprintf(output,SERVO_LIVESETUP, (servo_live==false?DIALOG_OFF:DIALOG_ON)); u8g2.print(output);
       u8g2.setCursor( 0, 10+(menuitem*13)); u8g2.print("*");
    } else {
       if ( menuitem != 0 ) {
          u8g2.setCursor(10, 10); sprintf(output,SERVO_PREVIOUS, value_old); u8g2.print(output);
       } else {
          u8g2.setCursor(10, 10); sprintf(output,SERVO_LIVESETUP, (servo_live==false?DIALOG_OFF:DIALOG_ON)); u8g2.print(output);
       }
       u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
       u8g2.drawGlyph(0, 10+(menuitem*13), 0x42);
       u8g2.setFont(u8g2_font_courB10_tf);
    }
    u8g2.sendBuffer();

    if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
         && (menuitem < 4 )
         && (value_change == false) ) {

         // debounce
         delay(10);
         while( digitalRead(SELECT_SW) == SELECT_LEVEL )
            ;
         delay(10);

         switch (menuitem) {
           case 0: initRotaries(SW_MENU, servo_live, 0, 1, 1);
                   break;
           case 1: initRotaries(SW_MENU, angle_min,  angle_hard_min, angle_fine,     1);
                   value_old = lastmin;
                   break;
           case 2: initRotaries(SW_MENU, angle_fine, angle_min,      angle_max,      1);
                   value_old = lastfine;
                   break;
           case 3: initRotaries(SW_MENU, angle_max,  angle_fine,     angle_hard_max, 1);
                   value_old = lastmax;
                   break;
         }
         value_change = true;
      }

      if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
           && (menuitem < 4 )
           && (value_change == true) ) {

         // debounce
         delay(10);
         while( digitalRead(SELECT_SW) == SELECT_LEVEL )
            ;
         delay(10);

         if ( servo_live == true )
           SERVO_WRITE(angle_min);
         initRotaries(SW_MENU, menuitem, 0, 4, -1);
         value_change = false;
      }

      if ( (digitalRead(SELECT_SW) == SELECT_LEVEL) && (menuitem == 4) ) {
        u8g2.setCursor(108, 10+(menuitem*13));
        u8g2.print(DIALOG_OK);
        u8g2.sendBuffer();
        delay(1000);
        i = 0;
      }
    }
}

void setupAutomatic(void) {
  int menuitem;
  int lastautostart     = autostart;
  int lastjartolerance  = jartolerance;
  int lastautocorrection = autocorrection;
  int lastoverfill        = overfill_gr;
  bool value_change = false;

  initRotaries(SW_MENU, 0, 0, 4, -1);

  u8g2.setFont(u8g2_font_courB10_tf);
  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH) {
      autostart     = lastautostart;
      autocorrection = lastautocorrection;
      overfill_gr     = lastoverfill;
      jartolerance  = lastjartolerance;
      return;
    }

    if ( value_change == false ) {
      menuitem = getRotariesValue(SW_MENU);
//      if ( menuitem == 3 )
//        menuitem = 4;  // An arrow in front of "Save"
    } else {
      switch (menuitem) {
        case 0: autostart     = getRotariesValue(SW_MENU);
                break;
        case 1: jartolerance  = getRotariesValue(SW_MENU);
                break;
        case 2: autocorrection = getRotariesValue(SW_MENU);
                break;
        case 3: overfill_gr     = getRotariesValue(SW_MENU);
                break;
      }
    }

    // Menu
    u8g2.clearBuffer();
    u8g2.setCursor(10, 10); sprintf(output,AUTO_AUTOSTART, (autostart==0?DIALOG_OFF:DIALOG_ON));     u8g2.print(output);
    u8g2.setCursor(10, 23); sprintf(output,AUTO_JARTOLERANCE, 177, jartolerance); u8g2.print(output);
    u8g2.setCursor(10, 36); sprintf(output,AUTO_AUTOCORRECTION, (autocorrection==0?DIALOG_OFF:DIALOG_ON)); u8g2.print(output);
    u8g2.setCursor(10, 49); sprintf(output,AUTO_OVERFILL, overfill_gr);                     u8g2.print(output);
    u8g2.setCursor(10, 62); u8g2.print(     DIALOG_SAVE);

      // Show position in menu. "*" when not selected, arrow when selected
    if ( value_change == false ) {
      u8g2.setCursor(0, 10+(menuitem*13)); u8g2.print("*");
    } else {
      u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
      u8g2.drawGlyph(0, 10+(menuitem*13), 0x42);
      u8g2.setFont(u8g2_font_courB10_tf);
    }
    u8g2.sendBuffer();

    // Selected menupoint to change
    if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
         && (menuitem < 4 )
         && (value_change == false) ) {

      // debounce
      delay(10);
      while( digitalRead(SELECT_SW) == SELECT_LEVEL )
        ;
      delay(10);

      switch (menuitem) {
        case 0: initRotaries(SW_MENU, autostart, 0, 1, 1);
                break;
        case 1: initRotaries(SW_MENU, jartolerance, 0, 99, 1);
                break;
        case 2: initRotaries(SW_MENU, autocorrection, 0, 1, 1);
                break;
        case 3: initRotaries(SW_MENU, overfill_gr, 0, 99, 1);
                break;
      }
      value_change = true;
    }

    // Taking changes in menupoint into account
    if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
         && (menuitem < 4 )
         && (value_change == true) ) {

      // debounce
      delay(10);
      while( digitalRead(SELECT_SW) == SELECT_LEVEL )
        ;
      delay(10);

      initRotaries(SW_MENU, menuitem, 0, 4, -1);
      value_change = false;
    }

        // Leave menu
    if ( (digitalRead(SELECT_SW) == SELECT_LEVEL) && (menuitem == 4) ) {
      u8g2.setCursor(108, 10+(menuitem*13));
      u8g2.print(DIALOG_OK);
      u8g2.sendBuffer();
      delay(1000);
      i = 0;
    }
  }
}

void setupFillquantity(void) {
  int j,k;
  int blinktime;
  initRotaries(SW_MENU, fquantity_index, 0, 4, -1);

  u8g2.setFont(u8g2_font_courB10_tf);
  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH)
      return;

    pos = getRotariesValue(SW_MENU);
    u8g2.clearBuffer();
    j = 0;
    while( j < 5  ) {
      u8g2.setCursor(10, 10+(j*13));
      sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
      u8g2.print(output);
      j++;
    }
    u8g2.setCursor(0, 10+(getRotariesValue(SW_MENU)*13));
    u8g2.print("*");
    u8g2.sendBuffer();

    if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) { // Fill quantity chosen
      delay(500);

//         initRotaries(SW_MENU, jars[pos].Weight, 30, 1000, 5);
      initRotaries(SW_MENU, weight2step( jars[pos].Weight) , 25, weight2step(MAXIMUMWEIGHT), 1);
      k = 1;
      while (k > 0){

        if ((digitalRead(button_stop_pin)) == HIGH)
          return;

        blinktime = (millis()/10) % 5;
//        jars[pos].Weight = getRotariesValue(SW_MENU);
        jars[pos].Weight = step2weight( getRotariesValue(SW_MENU) );
        u8g2.clearBuffer();

        j = 0;
        while( j < 5  ) {
          u8g2.setCursor(10, 10+(j*13));
          if (j == pos){
            if (blinktime < 3) {
              sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
            } else {
              sprintf(output, "%5s %3s","   ",JarTypeArray[jars[j].JarType]);
            }
          } else {
            sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
          }
          u8g2.print(output);
          j++;
        }
        u8g2.sendBuffer();

        if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) { // Weight confirmed
          delay(500);

          initRotaries(SW_MENU, jars[pos].JarType, 0, 2, 1);
          while (k > 0){
            if ((digitalRead(button_stop_pin)) == HIGH)
              return;
            blinktime = (millis()/10) % 5;
            jars[pos].JarType = getRotariesValue(SW_MENU);
            u8g2.clearBuffer();

            j = 0;
            while( j < 5  ) {
              u8g2.setCursor(10, 10+(j*13));
              if (j == pos){
                if (blinktime < 3) {
                  sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
                } else {
                  sprintf(output, "%4dg %3s",jars[pos].Weight,"  ");
                }
              } else {
                sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
              }
              u8g2.print(output);
              j++;
            }
            u8g2.sendBuffer();

            if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) { //Jar type confirmed
              u8g2.clearBuffer();
              j = 0;
              while( j < 5  ) {
                u8g2.setCursor(10, 10+(j*13));
                sprintf(output, "%4dg %3s", jars[j].Weight, JarTypeArray[jars[j].JarType]);
                u8g2.print(output);
                j++;
              }

              u8g2.setCursor(0, 10+(13*pos));
              u8g2.print("*");
              u8g2.sendBuffer();
              delay(1000);
              k = 0; //out

            }
          }
        }
      }
      fquantity = jars[pos].Weight;
      tare   = jars[pos].Tare;
      fquantity_index = pos;
      i = 0;
    }
  }
}

void setupParameter(void) {
  int menuitem;
  int lastbuzzer    = buzzermode;
  int lastsetup     = setup_modern;
  bool value_change = false;

  initRotaries(SW_MENU, 0, 0, 2, -1);

  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH) {
       buzzermode   = lastbuzzer;
       setup_modern = lastsetup;
       return;
    }

    if ( value_change == false ) {
      menuitem = getRotariesValue(SW_MENU);
      if ( menuitem == 2 )
        menuitem = 4;  // An arrow in front of "Save"
    } else {
      switch (menuitem) {
        case 0: buzzermode    = getRotariesValue(SW_MENU);
                break;
        case 1: setup_modern  = getRotariesValue(SW_MENU);
                break;
      }
    }

    // Menu
    u8g2.setFont(u8g2_font_courB10_tf);
    u8g2.clearBuffer();
    sprintf(output,PARAMETER_BUZZER, (buzzermode==0?DIALOG_OFF:DIALOG_ON));
    u8g2.setCursor(10, 10);    u8g2.print(output);
    sprintf(output,PARAMETER_MENU, (setup_modern==0?PARAMETER_LIST:PARAMETER_SCROLL));
    u8g2.setCursor(10, 23);    u8g2.print(output);
    u8g2.setCursor(10, 62);    u8g2.print(DIALOG_SAVE);

    // Show position in menu. "*" when not selected, arrow when selected
    if ( value_change == false ) {
       u8g2.setCursor(0, 10+((menuitem)*13)); u8g2.print("*");
    } else {
       u8g2.setCursor(0, 10+((menuitem)*13)); u8g2.print("-");
    }
    u8g2.sendBuffer();

    // Selected menupoint to change
    if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
         && (menuitem < 2 )
         && (value_change == false) ) {

         // debounce
         delay(10);
         while( digitalRead(SELECT_SW) == SELECT_LEVEL )
            ;
         delay(10);

         switch (menuitem) {
           case 0: initRotaries(SW_MENU, buzzermode, 0, 1, 1);
                   break;
           case 1: initRotaries(SW_MENU, setup_modern, 0, 1, 1);
                   break;
         }
         value_change = true;
      }

      // Taking changes in menupoint into account
      if ( (digitalRead(SELECT_SW) == SELECT_LEVEL)
           && (menuitem < 2 )
           && (value_change == true) ) {

         // debounce
         delay(10);
         while( digitalRead(SELECT_SW) == SELECT_LEVEL )
            ;
         delay(10);

         initRotaries(SW_MENU, menuitem, 0, 2, -1);
         value_change = false;
      }

          // Leave menu
      if ( (digitalRead(SELECT_SW) == SELECT_LEVEL) && (menuitem == 4) ) {
        u8g2.setCursor(108, 10+(menuitem*13));
        u8g2.print(DIALOG_OK);
        u8g2.sendBuffer();

        delay(1000);
        i = 0;
      }
    }
}

void setupClearPrefs(void) {
  initRotaries(SW_MENU, 1, 0, 1, -1);

  i = 1;
  while (i > 0) {
    if ((digitalRead(button_stop_pin)) == HIGH)
       return;

    pos = getRotariesValue(SW_MENU);
    u8g2.setFont(u8g2_font_courB10_tf);
    u8g2.clearBuffer();
    u8g2.setCursor(10, 12);    u8g2.print(CLEARPREFS_ERASE);
    u8g2.setCursor(10, 28);    u8g2.print(DIALOG_CANCEL);

    u8g2.setCursor(0, 12+((pos)*16));
    u8g2.print("*");
    u8g2.sendBuffer();

    if ((digitalRead(SELECT_SW)) == SELECT_LEVEL) {
      u8g2.setCursor(105, 12+((pos)*16));
      u8g2.print(DIALOG_OK);
      u8g2.sendBuffer();
      if ( pos == 0) {
        preferences.begin("EEPROM", false);
        preferences.clear();
        preferences.end();
        // Read deleted values, otherwise variables remain
        getPreferences();
      }
      delay(1000);
      i = 0;
    }
  }
}

void processSetup(void) {
  if ( setup_modern == 0 )
     processSetupList();
  else
     processSetupScroll();
}

void processSetupList(void) {
  if ( mode != MODE_SETUP ) {
     mode = MODE_SETUP;
     angle = angle_min;          // Close honeygate
     servo_enabled = 0;          // Servo deactivated
     SERVO_WRITE(angle);
     rotary_select = SW_MENU;
     initRotaries(SW_MENU, 0, 0, 9, -1);
  }

  int menuitem = getRotariesValue(SW_MENU);

  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  if( menuitem < 5 ) {
     u8g2.setCursor(10, 10);   u8g2.print(MENU_TARE);
     u8g2.setCursor(10, 23);   u8g2.print(MENU_CALIBRATION);
     u8g2.setCursor(10, 36);   u8g2.print(MENU_CORRECTION);
     u8g2.setCursor(10, 49);   u8g2.print(MENU_JARTYPES);
     u8g2.setCursor(10, 62);   u8g2.print(MENU_AUTO);
     u8g2.setFont(u8g2_font_open_iconic_arrow_2x_t);
     u8g2.drawGlyph(112, 64, 0x40);
  } else {
     u8g2.setCursor(10, 10);   u8g2.print(MENU_SERVO);
     u8g2.setCursor(10, 23);   u8g2.print(MENU_PARAMETERS);
     u8g2.setCursor(10, 36);   u8g2.print(MENU_COUNT);//Kud
     u8g2.setCursor(10, 49);   u8g2.print(MENU_COUNTTRIP);//Kud
     u8g2.setCursor(10, 62);   u8g2.print(MENU_RESETPREFS);
     u8g2.setFont(u8g2_font_open_iconic_arrow_2x_t);
     u8g2.drawGlyph(112, 16, 0x43);
  }
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(0, 10 + (((menuitem)%5) * 13));
  u8g2.print("*");
  u8g2.sendBuffer();

  if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) {
    // should prevent a button press from immediately selecting a submenu item
    delay(250);
    while( digitalRead(SELECT_SW) == SELECT_LEVEL ) {
    }
#ifdef isDebug
    Serial.print("Setup Position: ");
    Serial.println(menuitem);
#endif

    int lastpos = menuitem;
    if (menuitem == 0)   setupTare();              // Tare
    if (menuitem == 1)   setupCalibration();       // Calibration
    if (menuitem == 2)   setupCorrection();        // Correction
    if (menuitem == 3)   setupFillquantity();      // Fill quantity
    if (menuitem == 4)   setupAutomatic();         // Autostart/Autocorrection configuration
    if (menuitem == 5)   setupServoWinkel();       // Servo settings Minimum, Maximum und finedosage
    if (menuitem == 6)   setupParameter();         // Other parameters
    if (menuitem == 7)   setupCounter();           // Kud Counter
    if (menuitem == 8)   setupTripCounter();       // Kud Counter Trip
    setPreferences();

    if (menuitem == 9)   setupClearPrefs();        // Erase EEPROM
    initRotaries(SW_MENU, lastpos, 0, 9, -1);      // Menu parameters might have been changed
  }
}
void processSetupScroll(void) {
  if ( mode != MODE_SETUP ) {
     mode = MODE_SETUP;
     angle = angle_min;          // Close honeygate
     servo_enabled = 0;          // Servo deactivated
     SERVO_WRITE(angle);
     rotary_select = SW_MENU;
     initRotaries(SW_MENU, 124, 0,255, -1);
  }
  int MenuItemsCount = 10;
  const char *menuitems[] = {
    MENU_TARE,MENU_CALIBRATION,MENU_CORRECTION,MENU_JARTYPES,MENU_AUTO,MENU_SERVO,MENU_PARAMETERS,MENU_COUNT,MENU_COUNTTRIP,MENU_RESETPREFS
  };
  int menuitem = getRotariesValue(SW_MENU);
  menuitem = menuitem % MenuItemsCount;

  u8g2.clearBuffer();
  //Arrow up
  int abovepos = menuitem-1;
  if (menuitem == 0)
    abovepos = (MenuItemsCount-1);

  u8g2.setFont(u8g2_font_courB08_tf);
  u8g2.setCursor(30,12);
  u8g2.print(menuitems[abovepos]);

  //Arrow down
  int belowpos = menuitem+1;
  if (belowpos == MenuItemsCount)
    belowpos=0;
  u8g2.setCursor(30,62);
  u8g2.print(menuitems[belowpos]);

  //Arrow middle
  u8g2.drawLine(1, 20, 120, 20);
  u8g2.setFont(u8g2_font_courB12_tf);
  u8g2.setCursor(6, 38);
  u8g2.print(menuitems[menuitem]);
  u8g2.drawLine(1, 47, 120, 47);

  u8g2.sendBuffer();
  int lastpos = menuitem;

    if ( digitalRead(SELECT_SW) == SELECT_LEVEL ) {
    // should prevent a button press from immediately selecting a submenu item
    delay(250);
    while( digitalRead(SELECT_SW) == SELECT_LEVEL ) {}
#ifdef isDebug
    Serial.print("Setup Position: ");
    Serial.println(menuitem);
#endif

    int lastpos = menuitem;
   if (menuitem == 0)   setupTare();               // Tare
    if (menuitem == 1)   setupCalibration();       // Calibration
    if (menuitem == 2)   setupCorrection();        // Correction
    if (menuitem == 3)   setupFillquantity();      // Fill quantity
    if (menuitem == 4)   setupAutomatic();         // Autostart/Autocorrection configuration
    if (menuitem == 5)   setupServoWinkel();       // Servo settings Minimum, Maximum und finedosage
    if (menuitem == 6)   setupParameter();         // Other parameters
    if (menuitem == 7)   setupCounter();           // Kud Counter
    if (menuitem == 8)   setupTripCounter();       // Kud Counter Trip
    setPreferences();

    if (menuitem == 9)   setupClearPrefs();        // Erase EEPROM
    initRotaries(SW_MENU,lastpos, 0,255, -1);      // Menu parameters might have been changed
  }
}

void processAutomatic(void)
{
  int goalweight;            // Jar + correction
  long blinktime;
  static int autocorrection_gr = 0;
  int force_servo_enabled = 0;
  boolean full = false; //Kud

  static int weight_previous;    // Weight of the previously filled jar
  static int sample_count = 5;   // Number of identical measurements for dripping

  if ( mode != MODE_AUTOMATIC ) {
     mode = MODE_AUTOMATIC;
     angle = angle_min;              // Close honeygate
     servo_enabled = 0;              // Servo deactivated
     SERVO_WRITE(angle);
     auto_enabled = 0;               // start automatic filling
     tare_jar = 0;
     rotary_select = SW_ANGLE;       // Angle setting with rotatry
     initRotaries(SW_MENU, fquantity_index, 0, 4, 1);
     weight_previous = jars[fquantity_index].Weight + correction;
//     autocorrection_gr = 0;
  }

  pos = getRotariesValue(SW_ANGLE);
  // only regulate up to angle_fine, or solve via initRotaries?
  if ( pos < ((angle_fine*100)/angle_max) ) {
    pos = ((angle_fine*100)/angle_max);
    setRotariesValue(SW_ANGLE, pos);
  }

#ifdef USE_ROTARY                                                    // TODO: can the potentiometer change anything here?
  correction    = getRotariesValue(SW_CORRECTION);
  fquantity_index = getRotariesValue(SW_MENU);
#endif
  fquantity       = jars[fquantity_index].Weight;
  tare         = jars[fquantity_index].Tare;
  if ( tare <= 0 )
    auto_enabled = 0;

  // we only start if the tare for the jar type is set!
  // antoher push on the start button forces the servo activation
  if (((digitalRead(button_start_pin)) == HIGH) && (tare > 0)) {
    // debounce
    delay(10);
    while( digitalRead(button_start_pin) == HIGH )
       ;
    delay(10);

    if ( auto_enabled == 1 ) {
      force_servo_enabled = 1;
#ifdef isDebug
      Serial.println("force servo enabled");
#endif
    }
    auto_enabled    = 1;           // autofill activation
    rotary_select = SW_ANGLE;      // in case the start button is pushed during parameter changes
    setPreferences();              // in case parameters are changed using the rotary
  }

  if ((digitalRead(button_stop_pin)) == HIGH) {
    angle      = angle_min;
    servo_enabled = 0;
    auto_enabled  = 0;
    tare_jar   = 0;
//    autocorrection_gr = 0;
  }

  // Scale error correction in case the weight varies too much
#ifdef ERRORCORRECTION_SCALE
  int Comparisonweight = (int(SCALE_GETUNITS(SCALE_READS))) - tare;
  for (byte j = 0 ; j < 3; j++) { // Number of repetitions if deviation is too high
    weight = (int(SCALE_GETUNITS(SCALE_READS))) - tare;
    if (abs(weight - Comparisonweight ) < 50)  // Deviation for error detection
      break;
    delay(100);
  }
#else
  weight = (int(SCALE_GETUNITS(SCALE_READS))) - tare;
#endif

  // Jar removed -> close servo
  if (weight < -20) {
    angle      = angle_min;
    servo_enabled = 0;
    tare_jar   = 0;
    if ( autostart != 1 ) {  // Autostart not active
      auto_enabled  = 0;
    }
  }

// Automatic ON, empty jar set on, servo OFF -> fill jar
  if ((auto_enabled == 1) && (abs(weight) <= jartolerance) && (servo_enabled == 0)) {
    rotary_select = SW_ANGLE;     // in case a jar is set during paramater changes
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB24_tf);
    u8g2.setCursor(15, 43);
    u8g2.print(DIALOG_START);
    u8g2.sendBuffer();
    // wait briefly and check if the weight was not just a random fluctuation
    delay(1500);
    weight = (int(SCALE_GETUNITS(SCALE_READS))) - tare;

    if ( abs(weight) <= jartolerance ) {
      tare_jar   = weight;
#ifdef isDebug
      Serial.print("weight: ");            Serial.print(weight);
      Serial.print(" weight_previous: ");    Serial.print(weight_previous);
      Serial.print(" goalweight: ");       Serial.print(fquantity + correction + tare_jar + autocorrection_gr);
      Serial.print(" overfill: ");            Serial.print(overfill_gr);
      Serial.print(" Autocorrection: ");     Serial.println(autocorrection_gr);
#endif
      servo_enabled = 1;
      sample_count = 0;
      full = false; //Kud
      counted = false; //Kud
      buzzer(BUZZER_SHORT);
    }
  }
  goalweight = fquantity + correction + tare_jar + autocorrection_gr;

  // Adaptation of the autocorrection value
  if ( autocorrection == 1 )
  {
    if ( (auto_enabled == 1)                               // Automatic is enabled
       && (servo_enabled == 0 ) && (angle == angle_min)    // Honeygate is closed
       && (weight >= goalweight )                          // Jar is full
       && (sample_count <= 5)                              // drip amount not yet recorded
       ) {
    full = true;//Kud
    if ( (weight == weight_previous) && (sample_count < 5) ) {   // we want to see 5 times the same weight
      sample_count++;
    } else if ( weight != weight_previous ) {              // otherwise track the weight changes
      weight_previous = weight;
      sample_count = 0;
    } else if ( sample_count == 5 ) {                      // weight is 5 times identical, confirm autocorrection
      autocorrection_gr = (fquantity + overfill_gr + tare_jar) - (weight - autocorrection_gr);
      if ( correction + autocorrection_gr > overfill_gr ) {   // Autocorrection not allowed to overcorrect, max Fillquantity plus Overfill
        autocorrection_gr = overfill_gr - correction;
#ifdef isDebug
        Serial.print("Autocorrection limited to: ");
        Serial.println(autocorrection_gr);
#endif
      }
      buzzer(BUZZER_SUCCESS);
      sample_count++;                                      // correction value for this run has been reached
    }

    if ((full == true) && (counted == false)) { //Kud
      jars[fquantity_index].TripCount++;
      jars[fquantity_index].Count++;
//      setPreferences(); // this is done at the end
      counted = true;
    }
#ifdef isDebug
      Serial.print("Dripping:");
      Serial.print(" weight: ");        Serial.print(weight);
      Serial.print(" weight_previous: "); Serial.print(weight_previous);
      Serial.print(" sample_count: ");    Serial.print(sample_count);
      Serial.print(" Correction: ");      Serial.println(autocorrection_gr);
      Serial.print(" Counter Trip: ");    Serial.print(jars[fquantity_index].TripCount); //Kud
      Serial.print(" Counter: ");         Serial.println(jars[fquantity_index].Count); //Kud
#endif
    }
  }

  // Jar is partly filled up. Start will be forced through start button
  if ((auto_enabled == 1) && (weight > 5) && (force_servo_enabled == 1) ) {
    servo_enabled = 1;
    full = false; //Kud
    counted = false;//Kud
    buzzer(BUZZER_SHORT);
  }

  if (servo_enabled == 1) {
    angle = ((angle_max * pos) / 100);
  }

  if ((servo_enabled == 1) && (( goalweight - weight ) <= fine_dosage_weight)) {
    angle = ( ((angle_max*pos) / 100) * ((goalweight-weight) / fine_dosage_weight) );
  }

  if ((servo_enabled == 1) && (angle <= angle_fine)) {
    angle = angle_fine;
  }

 // Jar is full
  if ((servo_enabled == 1) && (weight >= goalweight)) {
    angle      = angle_min;
    servo_enabled = 0;

    if (counted == false) { //Kud
      jars[fquantity_index].TripCount++;
      jars[fquantity_index].Count++;
//      setPreferences();  this is done at the end
      counted = true;
    }
    if ( autostart != 1 )        // autostart is not active, no further start
      auto_enabled = 0;
    if ( autocorrection == 1 )   // autocorrection, weight saved
      weight_previous = weight;
    buzzer(BUZZER_SHORT);
  }

  SERVO_WRITE(angle);

#ifdef isDebug
#if isDebug >= 4
    Serial.print("Automatic:");
    Serial.print(" Weight: ");        Serial.print(weight);
    Serial.print(" Angle: ");         Serial.print(angle);
//    Serial.print(" Duration ");           Serial.print(millis() - scaletime);
//    Serial.print(" Jar Types: ");      Serial.print(fquantity);
//    Serial.print(" Correction: ");      Serial.print(correction);
//    Serial.print(" Tare_glas:");       Serial.print(tare_jar);
    Serial.print(" Autocorrection: ");  Serial.print(autocorrection_gr);
    Serial.print(" Goalweight ");     Serial.print(goalweight);
//    Serial.print(" Force servo: "); Serial.print(force_servo_enabled);
//    Serial.print(" servo_enabled ");     Serial.print(servo_enabled);
    Serial.print(" auto_enabled ");      Serial.println(auto_enabled);
#endif
#endif
  u8g2.clearBuffer();

  //Weight blinks if below the defined Fillquantity
// Correction factor and Fillquantity blink when adjusted via the rotary encoder
  blinktime = (millis()/10) % 5;

  // if no tare is set, the weight is not displayed, warning message is displayed
  if ( tare > 0 ) {
    // no jar on scale
    if ( weight < -20 ) {
      u8g2.setFont(u8g2_font_courB12_tf);
      u8g2.setCursor(28, 30); u8g2.print(SCREEN_NOJAR1);
      u8g2.setCursor(28, 44); u8g2.print(SCREEN_NOJAR2);
    } else {
      u8g2.setCursor(10, 42);
      u8g2.setFont(u8g2_font_courB24_tf);

      if( (autostart == 1) && (auto_enabled == 1 ) && (servo_enabled == 0) && (weight >= -5) && (weight - tare_jar < fquantity) && (blinktime < 2) ) {
        sprintf(output,"%5s", "     ");
      } else {
        sprintf(output,"%5dg", weight - tare_jar);
      }
      u8g2.print(output);
    }
  } else {
     u8g2.setCursor(42, 38);
     u8g2.setFont(u8g2_font_courB14_tf);
     sprintf(output,"%6s", SCREEN_NOTARE);
     u8g2.print(output);
  }

  // Play/Pause icon, wether the automatic filling is active or not
  u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
  u8g2.drawGlyph(0, 40, (auto_enabled==1)?0x45:0x44 );

  u8g2.setFont(u8g2_font_courB12_tf);
  // Arrow up, oepning angle value and percentage, autostart sign
  u8g2.setCursor(0, 11);
  sprintf(output,SCREEN_SHORT1, angle, (autostart==1)?SCREEN_SHORT2:"  ", pos);
  u8g2.print(output);

  u8g2.setFont(u8g2_font_courB10_tf);
  // Arrow down, selected value to be changed blinking.
// Adjustment only when automatic mode is inactive, controlled via interrupt function
  if(servo_enabled == 1) {
    int progressbar = 128.0*((float)weight/(float)goalweight);
    progressbar = constrain(progressbar,0,128);

    u8g2.drawFrame(0, 50, 128, 14 );
    u8g2.drawBox  (0, 50, progressbar, 14 );
  }
  else
  {
    if( autocorrection == 1 ){
      u8g2.setCursor( 0, 64);
      u8g2.print(SCREEN_SHORT3);
      u8g2.setCursor(10, 64);
    } else {
      u8g2.setCursor( 0, 64);
    }

    if( rotary_select == SW_CORRECTION && blinktime < 2 ) {
      if (jars[fquantity_index].Weight > 999){
        sprintf(output,SCREEN_SHORT4A,(autocorrection==1)?"":" ", "1kg", JarTypeArray[jars[fquantity_index].JarType]  );
      } else {
        sprintf(output,SCREEN_SHORT4B,(autocorrection==1)?"":" ", jars[fquantity_index].Weight, JarTypeArray[jars[fquantity_index].JarType] );
      }
    } else if ( rotary_select == SW_MENU && blinktime < 2 ) {
        sprintf(output,SCREEN_SHORT4C , correction + autocorrection_gr);
    } else {
      if (jars[fquantity_index].Weight > 999){
        sprintf(output,SCREEN_SHORT4D, correction + autocorrection_gr, (autocorrection==1)?"":" ", "1kg", JarTypeArray[jars[fquantity_index].JarType] );
      } else {
        sprintf(output,SCREEN_SHORT4E, correction + autocorrection_gr, (autocorrection==1)?"":" ", jars[fquantity_index].Weight, JarTypeArray[jars[fquantity_index].JarType] );
      }
    }
    u8g2.print(output);
  }

  u8g2.sendBuffer();

  setPreferences();
}

void processManualmode(void)
{
  static unsigned long scaletime;
  static unsigned long duration;

  if ( mode != MODE_MANUAL ) {
     mode = MODE_MANUAL;
     angle = angle_min;              // Close honeygate
     servo_enabled = 0;              // Servo deactivated
     SERVO_WRITE(angle);
     rotary_select = SW_ANGLE;
     tare = 0;
  }

  pos = getRotariesValue(SW_ANGLE);
  weight = SCALE_GETUNITS(SCALE_READS) - tare;

  if ((digitalRead(button_start_pin)) == HIGH) {
    servo_enabled = 1;
  }

  if ((digitalRead(button_stop_pin)) == HIGH) {
    servo_enabled = 0;
  }

#ifdef USE_ROTARY_SW
  if ( ((digitalRead(outputSW)) == LOW) /*&& (tare == 0) */ ) {  // otherwise the button must be debounced!
      tare = SCALE_GETUNITS(SCALE_READS);
  }
#endif

  if (servo_enabled == 1) {
    angle = ((angle_max * pos) / 100);
  } else {
    angle = angle_min;
  }
  angle = constrain(angle, angle_min, angle_max);
  SERVO_WRITE(angle);

#ifdef isDebug
#if isDebug >= 4
    Serial.print("Manual mode:");
    Serial.print(" Weight ");     Serial.print(weight);
    Serial.print(" Angle ");      Serial.print(angle);
    Serial.print(" Duration ");       Serial.print(millis() - scaletime);
    Serial.print(" servo_enabled "); Serial.println(servo_enabled);
#endif
#endif
  scaletime = millis();
  // OLED output. Duration is around 170ms
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_courB24_tf);
  u8g2.setCursor(10, 42);
  sprintf(output,"%5dg", weight);
//  sprintf(output,"%5dg", duration);
  u8g2.print(output);

  u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
  u8g2.drawGlyph(0, 40, (servo_enabled==1)?0x45:0x44 );

  u8g2.setFont(u8g2_font_courB12_tf);
  u8g2.setCursor(0, 11);
  sprintf(output,SCREEN_SHORT7, angle, pos);
  u8g2.print(output);
  u8g2.setCursor(0, 64);
  sprintf(output, SCREEN_SHORT5, (tare>0?SCREEN_SHORT6:"    "));
  u8g2.print(output);

  u8g2.sendBuffer();

  setPreferences();
//  u8g2.updateDisplayArea(4,2,12,6);  // faster but les precise Displayoutput.
  duration = millis() - scaletime;
}

void setup()
{
  // enable internal pull downs for digital inputs
  pinMode(button_start_pin, INPUT_PULLDOWN);
  pinMode(button_stop_pin, INPUT_PULLDOWN);
  pinMode(switch_mode_pin, INPUT_PULLDOWN);
  pinMode(switch_setup_pin, INPUT_PULLDOWN);
#if HARDWARE_LEVEL == 2
  pinMode(vext_ctrl_pin, INPUT_PULLDOWN);
#endif

  Serial.begin(115200);
  while (!Serial) {
  }
#ifdef isDebug
    Serial.println("HaniMandl Start");
#endif

// Rotary
#ifdef USE_ROTARY_SW
  pinMode(outputSW, INPUT_PULLUP);
  attachInterrupt(outputSW, isr1, FALLING);
#endif
#ifdef USE_ROTARY
  pinMode(outputA,INPUT);
  pinMode(outputB,INPUT);
  attachInterrupt(outputA, isr2, CHANGE);
#endif

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

// Buzzer
  pinMode(buzzer_pin, OUTPUT);

// short delay to let chip power up
  delay (100);

// Read preferences from EEPROM
  getPreferences();

  // Servo initialization and closing
#ifdef SERVO_EXTENDED
  servo.attach(servo_pin,  750, 2500); // extended initialization, does not control every servo
#else
  servo.attach(servo_pin, 1000, 2000); // default values. Warning, drives the zero point very far!
#endif
  SERVO_WRITE(angle_min);

// Scale identification we do this before the boot screen to let it warm up for 3 seconds
scale.begin(hx711_dt_pin, hx711_sck_pin);
  if (scale.wait_ready_timeout(1000)) {               // Scale connected?
    scale.power_up();
    scale_present = 1;
#ifdef isDebug
      Serial.println("Scale detected");
#endif
  }

// Boot Screen
  u8g2.setBusClock(800000);   // experimental
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();
  print_logo();
  buzzer(BUZZER_SHORT);
  delay(2000);
  print_credits();
  delay(3000);
  print_credits2();
  delay(2000);

// Scale setup, setting scaling factor
  if (scale_present ==1 ) {                           // Scale connected?
    if ( factor == 0 ) {                              // Present but uncalibrated
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.setCursor( 10, 24); u8g2.print(STARTUP_NOTCAL1);
      u8g2.setCursor( 10, 56); u8g2.print(STARTUP_NOTCAL2);
      u8g2.sendBuffer();
#ifdef isDebug
      Serial.println("Scale not calibrated");
#endif
      delay(2000);
    } else {                                          // Tare and scaling setup
      scale.set_scale(factor);
      scale.set_offset(long(weight_empty));
#ifdef isDebug
      Serial.println("Scale initialized");
#endif
    }
  } else {                                            // No scale connected
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_courB14_tf);
    u8g2.setCursor( 10, 24); u8g2.print(STARTUP_NOSCALE1);
    u8g2.setCursor( 10, 56); u8g2.print(STARTUP_NOSCALE2);
    u8g2.sendBuffer();
    buzzer(BUZZER_ERROR);
#ifdef isDebug
    Serial.println("No scale!");
#endif
    delay(2000);
  }

// initial calibration of empty weight because of temperature variations
// If more than 20g deviation, there is probably something on the scale.
  if (scale_present == 1) {
    weight = SCALE_GETUNITS(SCALE_READS);
    if ( (weight > -20) && (weight < 20) ) {
      scale.tare(10);
      buzzer(BUZZER_SUCCESS);
#ifdef isDebug
      Serial.print("Tare adjusted to: ");
      Serial.println(weight);
#endif
    } else if (factor != 0) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_courB14_tf);
      u8g2.setCursor( 10, 24); u8g2.print(STARTUP_WEIGHTON1);
      u8g2.setCursor( 10, 56); u8g2.print(STARTUP_WEIGHTON2);
      u8g2.sendBuffer();
#ifdef isDebug
        Serial.print("Weight on the scale: ");
        Serial.println(weight);
#endif
      delay(5000);

      // New try, in case the weight is taken off the scale
      weight = SCALE_GETUNITS(SCALE_READS);
      if ( (weight > -20) && (weight < 20) ) {
        scale.tare(10);
        buzzer(BUZZER_SUCCESS);
#ifdef isDebug
        Serial.print("Tare adjusted to: ");
        Serial.println(weight);
#endif
      } else {    // Play warning sound
        buzzer(BUZZER_LONG);
      }
    }
  }

// initialization of the 3 rotary datastructures
  initRotaries(SW_ANGLE,    0,   0, 100, 5 );       // Angle
  initRotaries(SW_CORRECTION, 0, -90,  20, 1 );     // Correction
  initRotaries(SW_MENU,      0,   0,   7, 1 );      // Menu choices

// Setting up rotary parameters from the preferences
  setRotariesValue(SW_ANGLE,    pos);
  setRotariesValue(SW_CORRECTION, correction);
  setRotariesValue(SW_MENU,      fquantity_index);
}

void loop()
{
  rotating = true;     // debounce Management

  // Setup menu
  if ((digitalRead(switch_setup_pin)) == HIGH)
    processSetup();

  // Automatic operation
  if ((digitalRead(switch_mode_pin)) == HIGH)
    processAutomatic();

  // Manual operation
  if ((digitalRead(switch_mode_pin) == LOW)
      && (digitalRead(switch_setup_pin) == LOW))
    processManualmode();
}

void print_credits() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.setCursor(0, 10);    u8g2.print("Idea: M. Vasterling");
  u8g2.setCursor(0, 23);    u8g2.print("Code: M. Vasterling, M.");
  u8g2.setCursor(0, 36);    u8g2.print("Wetzel, C. Gruber, A.");
  u8g2.setCursor(0, 49);    u8g2.print("Holzhammer, M. Junker,");
  u8g2.setCursor(0, 62);    u8g2.print("J. Kuder, J. Bruker");
  u8g2.sendBuffer();
}

void print_credits2() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.setCursor(0, 10);    u8g2.print("More contributors:");
  u8g2.setCursor(0, 23);    u8g2.print("A. Motl, J. Lehmann");
  u8g2.setCursor(0, 36);    u8g2.print("");
  u8g2.setCursor(0, 49);    u8g2.print("");
  u8g2.setCursor(0, 62);    u8g2.print("");
  u8g2.sendBuffer();
}

void print_logo() {
  const unsigned char logo_biene1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0,
  0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC1, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xF8, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x3F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x70, 0x00, 0xF0, 0xFF, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x80, 0xFF, 0xFF, 0x0F, 0x00, 0xFF, 0xFF, 0x80, 0xF1, 0x47, 0xF0, 0x07, 0x00, 0x3E, 0xE0, 0xFF, 0xFF, 0x07,
  0xF9, 0x07, 0x7E, 0x00, 0x00, 0x78, 0xF0, 0x03, 0xE0, 0x1F, 0xF8, 0x07, 0x1F, 0x00, 0x00, 0x70, 0x3C, 0x00, 0x00, 0xFE, 0x38, 0xC0, 0x03, 0x00,
  0x00, 0xF0, 0x0E, 0x00, 0x00, 0xF8, 0x03, 0xF8, 0x00, 0x00, 0x00, 0xE0, 0x06, 0x00, 0x00, 0xC0, 0x0F, 0x7C, 0x00, 0x00, 0x00, 0xE0, 0x06, 0x00,
  0x00, 0x00, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x70, 0x03, 0x00, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00, 0x00, 0x70, 0x03, 0x00, 0x00, 0x00, 0xF0, 0x03,
  0x00, 0x00, 0x00, 0x38, 0x03, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x1C, 0x07, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0x07, 0x00, 0x00, 0x0F,
  0x0F, 0x00, 0x00, 0x78, 0x78, 0xE0, 0x3F, 0x00, 0xC0, 0x07, 0x3E, 0x00, 0x80, 0xFF, 0x3C, 0xC0, 0x7F, 0x00, 0xF0, 0x01, 0xFC, 0x00, 0xE0, 0xFF,
  0x1C, 0x80, 0xFF, 0x01, 0x7E, 0x00, 0xF0, 0xFF, 0xFF, 0x3F, 0x0E, 0x00, 0xFE, 0xFF, 0x0F, 0x00, 0xC0, 0xFF, 0xFF, 0x07, 0x0F, 0x00, 0xC0, 0x1F,
  0x00, 0x00, 0x00, 0xFC, 0x3F, 0x00, 0x07, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x03, 0xE0, 0x00, 0x70, 0x00, 0x00, 0x00, 0xC0,
  0x01, 0xC0, 0x03, 0xC0, 0x01, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x00, 0xE0, 0x81, 0xC3, 0x01, 0xC0, 0x01, 0x00, 0x00, 0x70, 0x00, 0xE0, 0xF1, 0x8F,
  0x03, 0x80, 0x03, 0x00, 0x00, 0x38, 0x00, 0xF0, 0xFC, 0x9F, 0x07, 0x00, 0x07, 0x00, 0x00, 0x1C, 0x00, 0xF8, 0x1C, 0x1C, 0x0F, 0x00, 0x06, 0x00,
  0x00, 0x1C, 0x00, 0xFE, 0x00, 0x00, 0x1F, 0x00, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0xF7, 0x00, 0x00, 0x7F, 0x00, 0x0C, 0x00, 0x00, 0x06, 0x80, 0x73,
  0x00, 0x00, 0xE6, 0x00, 0x0C, 0x00, 0x00, 0x07, 0xE0, 0x71, 0x00, 0x00, 0xC6, 0x03, 0x0C, 0x00, 0x00, 0x07, 0x70, 0x70, 0xF0, 0x0F, 0x86, 0x07,
  0x0C, 0x00, 0x00, 0x03, 0x3C, 0x70, 0xFC, 0x3F, 0x06, 0x1F, 0x0E, 0x00, 0x00, 0x03, 0x1E, 0x70, 0xFE, 0x3F, 0x06, 0xFC, 0x07, 0x00, 0x00, 0x87,
  0x0F, 0x70, 0x1E, 0x38, 0x06, 0xF0, 0x03, 0x00, 0x00, 0xFE, 0x03, 0xF0, 0x00, 0x00, 0x06, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0x00, 0xF0, 0x00, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x80, 0x03, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0x0F, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xE0, 0xF1, 0x9F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3B, 0x9C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
  0x07, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x7C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x0D,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  u8g2.clearBuffer();
  u8g2.drawXBM(0,0,80,64,logo_biene1);
  u8g2.setFont(u8g2_font_courB14_tf);
  u8g2.setCursor(85, 27);    u8g2.print("HANI");
  u8g2.setCursor(75, 43);    u8g2.print("MANDL");
  u8g2.setFont(u8g2_font_courB08_tf);
  u8g2.setCursor(77, 64);    u8g2.print("v.0.3.0");
  u8g2.sendBuffer();
}

// We use an active buzzer, this eliminates the need for the tone library, which interacts with the servo anyway.
void buzzer(byte type) {
  if (buzzermode == 1) {
    switch (type) {
      case BUZZER_SHORT: //short
        digitalWrite(buzzer_pin,HIGH);
        delay(100);
        digitalWrite(buzzer_pin,LOW);
        break;

      case BUZZER_LONG: //long
        digitalWrite(buzzer_pin,HIGH);
        delay(500);
        digitalWrite(buzzer_pin,LOW);
        break;

      case BUZZER_SUCCESS: //success
        digitalWrite(buzzer_pin,HIGH);
        delay(100);
        digitalWrite(buzzer_pin,LOW);
        delay(100);
        digitalWrite(buzzer_pin,HIGH);
        delay(100);
        digitalWrite(buzzer_pin,LOW);
        delay(100);
        digitalWrite(buzzer_pin,HIGH);
        delay(100);
        digitalWrite(buzzer_pin,LOW);
        break;

      case BUZZER_ERROR: //error
        digitalWrite(buzzer_pin,HIGH);
        delay(1500);
        digitalWrite(buzzer_pin,LOW);
        break;
    }
  }
}

// Support functions for stepwise weight adjustement
int step2weight( int step ) {
  int sum = 0;

  if ( step > 210 ) { sum += (step-210)*1000; step -= (step-210); }
  if ( step > 200 ) { sum += (step-200)* 500; step -= (step-200); }
  if ( step > 160 ) { sum += (step-160)* 100; step -= (step-160); }
  if ( step > 140 ) { sum += (step-140)*  25; step -= (step-140); }
  if ( step >  50 ) { sum += (step- 50)*   5; step -= (step- 50); }
  sum += step;

  return sum;
}
int weight2step ( int sum ) {
  int step = 0;

  if ( sum > 10000 ) { step += (sum-10000)/1000; sum -= ((sum-10000)); }
  if ( sum >  5000 ) { step += (sum-5000)/500;   sum -= ((sum-5000)); }
  if ( sum >  1000 ) { step += (sum-1000)/100;   sum -= ((sum-1000)); }
  if ( sum >   500 ) { step += (sum-500)/25;     sum -= ((sum-500));  }
  if ( sum >    50 ) { step += (sum-50)/5;       sum -= (sum-50);   }
  step += sum;

  return step;
}

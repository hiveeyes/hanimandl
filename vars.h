// Rotary Encoder
const int outputA  = 33;
const int outputB  = 26;
const int outputSW = 32;

// Servo
const int servo_pin = 2;

// 3x Schalter Ein 1 - Aus - Ein 2
#if HARDWARE_LEVEL == 1
const int switch_betrieb_pin = 19;
const int switch_vcc_pin     = 22;     // <- Vcc
const int switch_setup_pin   = 21;
#elif HARDWARE_LEVEL == 2
const int switch_betrieb_pin = 23;
const int switch_vcc_pin     = 19;     // <- Vcc
const int switch_setup_pin   = 22;
const int vext_ctrl_pin      = 21;     // Vext control pin
#elif HARDWARE_LEVEL == 3
// Platine Sebastian: ESP32-WROOM + 2.24" OLED SPI
const int switch_betrieb_pin = 19; //23;
const int switch_vcc_pin     = 13;     // <- Vcc 
const int switch_setup_pin   = 27;//22;
//const int vext_ctrl_pin      = 18;     // Vext control pin
#else
#error Hardware Level nicht definiert! Korrektes #define setzen!
#endif

// Taster 
const int button_start_vcc_pin = 13;  // <- Vcc 
const int button_start_pin     = 12;
const int button_stop_vcc_pin  = 13;  // <- Vcc 
const int button_stop_pin      = 14;

// Poti
const int poti_pin = 39;

// Wägezelle-IC HX711
#if WEIGHT_TYPE == 0
const int hx711_sck_pin = 17;
const int hx711_dt_pin  = 4;
#endif

// rs232  Protokoll TEMstandard    AKTUELL NICHT LAUFFÄHIG
/* Notizen: 
-----------
Rohformat:
    15:08:06.631 -> I          0B9
    15:08:06.631 -> +   137.5kg 7B       <- Gemessenes Gewicht. +- fehlt, wenn die Messung nicht stabil ist.
    15:08:06.667 -> t       0kg 76       <- Taragewicht

serbuf[0] = I
serbuf[16] = +,- oder " "
atof(&serbuf[17]) = das Gewicht
serbuf[32] = t
*/
#if WEIGHT_TYPE == 1      
const int rs232Timeout = 210;  // maximale Wartezeit, um eine Antwort von der Waage zu erhalten 
const int maxWeightAge = 450;  // Gewichtsmessungen die älter als maxWeightAge sind, werden auf -999g gesetzt
const int rs232wait = 50;      // erzwungene Wartezeit, bevor eine neue Gewichtsabfrage gestellt werden kann
const byte rs232request[] = {87,13,10};  // rs232-Befehl, um das Gewicht anzufordern: <w><CR><LF>
const int RXD2 = 17;
const int TXD2 = 4;
#endif
// rs232 Protokoll Delta-Cyprus
#if WEIGHT_TYPE == 2
const int RXD2 = 17;           // receive Pin des max3232
const int TXD2 = 4;            // send Pin des max3232 
const int rs232Timeout = 210;  // maximale Wartezeit, um eine Antwort von der Waage zu erhalten 
const int maxWeightAge = 450;  // Gewichtsmessungen die älter als maxWeightAge sind, werden auf -999g gesetzt
const int rs232wait = 50;      // erzwungene Wartezeit, bevor eine neue Gewichtsabfrage gestellt werden kann
const byte rs232request[] = {68,13,10};  // rs232-Befehl, um das Gewicht anzufordern: <d><CR><LF>
#endif

// Buzzer - aktiver Piezo
static int buzzer_pin = 25;

Servo servo;
#if WEIGHT_TYPE == 0
HX711 scale;
#endif
Preferences preferences;

// Datenstrukturen für Rotary Encoder
struct rotary {                        
  int Value;
  int Minimum;
  int Maximum;
  int Step;
};
#define SW_WINKEL    0
#define SW_KORREKTUR 1
#define SW_MENU      2
struct rotary rotaries[3];         // Werden in setup() initialisiert
int rotary_select = SW_WINKEL;
static boolean rotating = false;   // debounce management für Rotary Encoder


//MarcN: gemessenes Gewicht mit timestamp für rs232 Waagen
struct rs232weight_struct {
  int weight;
  unsigned long timestamp;
  float flow;                   // Fließgeschwindigkeit g/sec
  unsigned long delta;          // Abstand in ms zwischen zwei Messungen 
};

// Füllmengen für 5 verschiedene Gläser
struct glas { 
  int Gewicht;
  int GlasTyp;    //JB
  int Tara;
  int TripCount;  //Kud
  int Count;      //Kud
};
const char *GlasTypArray[3] = { "DIB", "TOF", "DEE"};//DIB = DeutscherImkerBund-Glas DEE= DeepTwist-Glas TOF= TwistOff-Glas //JB
struct glas glaeser[5] =            { 
                                         {  125, 0, -9999, 0, 0 },
                                         {  250, 1, -9999, 0, 0 },
                                         {  250, 2, -9999, 0, 0 },
                                         {  500, 1, -9999, 0, 0 },
                                         {  500, 0, -9999, 0, 0 } 
                                    };

// Allgemeine Variablen
boolean isr2running = false;            // StatusFlag, damit ISR2 nur exklusiv und nicht parallel läuft
char serbuf[260];               // MarcN: Serieller Buffer ist 256 Bytes
int serlen;                     // MarcN: Anzahl der Zeichen im rs232 Puffer
rs232weight_struct rs232weight; // MarcN: das letzte über rs232 gelesene Geicht mit timestamp in ms
unsigned long ageRefresh;
int tmpWeightShow;
int i;                          // allgemeine Zählvariable
int pos;                        // aktuelle Position des Poti bzw. Rotary 
int gewicht;                    // aktuelles Gewicht
int tara;                       // Tara für das ausgewählte Glas, für Automatikmodus
int tara_glas;                  // Tara für das aktuelle Glas, falls Glasgewicht abweicht
long gewicht_leer;              // Gewicht der leeren Waage
float faktor;                   // Skalierungsfaktor für Werte der Waage
int fmenge;                     // ausgewählte Füllmenge
int fmenge_index;               // Index in gläser[]
int korrektur;                  // Korrekturwert für Abfüllmenge
int autostart;                  // Vollautomatik ein/aus
int autokorrektur;              // Autokorrektur ein/aus
int kulanz_gr;                  // gewollte Überfüllung im Autokorrekturmodus in Gramm
int winkel;                     // aktueller Servo-Winkel
int winkel_hard_min = 0;        // Hard-Limit für Servo
int winkel_hard_max = 180;      // Hard-Limit für Servo
int winkel_min = 0;             // konfigurierbar im Setup
int winkel_max = 85;            // konfigurierbar im Setup
int winkel_fein = 35;           // konfigurierbar im Setup
float fein_dosier_gewicht = 60; // float wegen Berechnung des Schliesswinkels
int servo_aktiv = 0;            // Servo aktivieren ja/nein
int kali_gewicht = 500;         // frei wählbares Gewicht zum kalibrieren
char ausgabe[30];               // Fontsize 12 = 13 Zeichen maximal in einer Zeile
int modus = -1;                 // Bei Modus-Wechsel den Servo auf Minimum fahren
int auto_aktiv = 0;             // Für Automatikmodus - System ein/aus?
int waage_vorhanden = 0;        // HX711 nicht ansprechen, wenn keine Waage angeschlossen ist, sonst Crash
long preferences_chksum;        // Checksumme, damit wir nicht sinnlos Prefs schreiben
int buzzermode = 0;             // 0 = aus, 1 = ein. TODO: Tastentöne als buzzermode 2?
bool gezaehlt = true;           // Kud Zähl-Flag
bool setup_modern = 1;          // Setup als rotierendes Menu   
int glastoleranz = 20;          // Gewicht für autostart darf um +-20g schwanken, insgesamt also 40g!

TaskHandle_t rs232readerTaskCore0;   // MarcN: Thread auf Kern0. Liest den rs232 Puffer permanant aus
TaskHandle_t rotarySpinTaskCore0;    // MarcN: Thread auf Kern0. Verarbeitet die Drehungen des Rotary. Alternativ zu ISR2(); 

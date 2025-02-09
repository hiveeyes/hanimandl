// NOTES:
    // Here you can define all the text used in the program.
    // Characters numbers and spacing should be kept the same to avoid display issues. (espacially when code is present)
    // If the text is too long, it might be cut off or superimposed.
    // If the code is broken, the display might be unreadable and the programm might not work at all.
    // Changes must be tested to check for messages integrity and functionality. Languages singularities must be taken into account.
    // If a change is made to the general structure i.e. adding a new box,
    // the corresponding change must be made in every resources_*.h file to avoid breaking the code in other languages.


//  Names of menus
    // (in both the list and scroll menu types)
#define MENU_TARE           "Tara"
#define MENU_CALIBRATION    "Kalibrieren"
#define MENU_CORRECTION     "Korrektur"
#define MENU_JARTYPES       "Füllmenge"
#define MENU_AUTO           "Automatik"
#define MENU_SERVO          "Servowinkel"
#define MENU_PARAMETERS     "Parameter"
#define MENU_COUNT          "Zähler"
#define MENU_COUNTTRIP      "Zähler Trip"
#define MENU_RESETPREFS     "Clear Prefs"

//  Various dialog messages used in different places
#define DIALOG_RESET        "Reset"
#define DIALOG_CANCEL       "Abbrechen"
#define DIALOG_OK           "OK"
#define DIALOG_MISSING      " Fehlt"
#define DIALOG_SAVE         "Speichern"
#define DIALOG_START        "START"
#define DIALOG_ON           "ein"
#define DIALOG_OFF          "aus"

// Messages for various menu screens
#define CALIBRATION_PROMPT1 "Bitte Waage"
#define CALIBRATION_PROMPT2 "leeren"
#define CALIBRATION_PROMPT3 "und mit OK"
#define CALIBRATION_PROMPT4 "bestätigen"
#define CALIBRATION_PROMPT5 "Bitte "
#define CALIBRATION_PROMPT6 "aufstellen"
#define CORRECTION_CORR     "Korrektur"
#define CORRECTION_PREVIOUS "alter Wert"
#define SERVO_MINIMUM       "Minimum   %3d"     //Do not break the code!!
#define SERVO_MAXIMUM       "Maximum   %3d"     //Do not break the code!!
#define SERVO_FINE          "Feindos.  %3d"     //Do not break the code!!
#define SERVO_LIVESETUP     "Livesetup %3s"     //Do not break the code!!
#define SERVO_PREVIOUS      "previous: %3d"     //Do not break the code!!
#define AUTO_AUTOSTART      "Autostart %3s"     //Do not break the code!!
#define AUTO_JARTOLERANCE   "Glastol. %c%2dg"     //Do not break the code!!
#define AUTO_AUTOCORRECTION "Autokorr. %3s"     //Do not break the code!!
#define AUTO_OVERFILL       "-> Kulanz %2dg"     //Do not break the code!!
#define PARAMETER_BUZZER    "Buzzer    %3s"     //Do not break the code!!
#define PARAMETER_MENU      "Menü   %6s"     //Do not break the code!!
#define PARAMETER_LIST      " List"
#define PARAMETER_SCROLL    "Scroll"
#define CLEARPREFS_ERASE    "Löschen"
#define COUNT_DISPLAY1      "%4dg %3s"     //Do not break the code!!
#define COUNT_DISPLAY2      "%5d Pcs."     //Do not break the code!!
#define COUNT_DISPLAY3      "Summe Trip:"
#define COUNT_DISPLAY4      "Summe:"
#define JARTYPE_1           "DIB"
#define JARTYPE_2           "TOF"
#define JARTYPE_3           "DEE"

// Elements of the manual and automatic screen display
#define SCREEN_NOJAR1       "Bitte Glas"
#define SCREEN_NOJAR2       "aufstellen"
#define SCREEN_NOTARE       "no tara!"
#define SCREEN_SHORT1       "W=%-3d %2s %3d%%"     //Do not break the code!!
#define SCREEN_SHORT2       "AS"
#define SCREEN_SHORT3       "a"
#define SCREEN_SHORT4A      "k=   %s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4B      "k=   %s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT4C      "k=%-3d"     //Do not break the code!!
#define SCREEN_SHORT4D      "k=%-3d%s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4E      "k=%-3d%s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT5       "Manuell  %s"     //Do not break the code!!
#define SCREEN_SHORT6       "Tara"
#define SCREEN_SHORT7       "W=%-3d    %3d%%"     //Do not break the code!!

// Error messages at startup
#define STARTUP_NOTCAL1     "  Nicht"
#define STARTUP_NOTCAL2     "kalibriert"
#define STARTUP_NOSCALE1    "  Keine"
#define STARTUP_NOSCALE2    "  Waage!"
#define STARTUP_WEIGHTON1   "   Waage"
#define STARTUP_WEIGHTON2   "  leeren!"

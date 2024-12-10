// NOTES:
    // Here you can define all the text used in the program.
    // Characters numbers and spacing should be kept the same to avoid display issues. (espacially when code is present)
    // If the text is too long, it might be cut off or superimposed.
    // If the code is broken, the display might be unreadable and the programm might not work at all.
    // Changes must be tested to check for messages integrity and functionality. Languages singularities must be taken into account.
    // If a change is made to the general structure i.e. adding a new box,
    // the corresponding change must be made in every resources_*.h file to avoid breaking the code in other languages.


//  Names of menus
    // (in the list and scroll menu types)
#define MENU_TARE           "Tare values"
#define MENU_CALIBRATION    "Calibration"
#define MENU_CORRECTION     "Correction"
#define MENU_JARTYPES       "Jar types"
#define MENU_AUTO           "Automatic"
#define MENU_SERVO          "Servo"
#define MENU_PARAMETERS     "Parameters"
#define MENU_COUNT          "Count"
#define MENU_COUNTTRIP      "Trip count"
#define MENU_RESETPREFS     "Reset Prefs"

//  Various dialog messages used in different places
#define DIALOG_RESET        "Reset"
#define DIALOG_CANCEL       "Cancel"
#define DIALOG_OK           "OK"
#define DIALOG_MISSING      "Missing"
#define DIALOG_SAVE         "Save"
#define DIALOG_START        "START"
#define DIALOG_ON           "ON"
#define DIALOG_OFF          "OFF"

// Messages for various menu screens
#define CALIBRATION_PROMPT1 "Please empty"
#define CALIBRATION_PROMPT2 "the scale"
#define CALIBRATION_PROMPT3 "and confirm"
#define CALIBRATION_PROMPT4 "with OK"
#define CALIBRATION_PROMPT5 "Pls. put "
#define CALIBRATION_PROMPT6 "on the scale"
#define CORRECTION_CORR     "Correction"
#define CORRECTION_PREVIOUS "previous value"
#define SERVO_MINIMUM       "Minimum   %3d"     //Do not break the code!!
#define SERVO_MAXIMUM       "Maximum   %3d"     //Do not break the code!!
#define SERVO_FINE          "Finedos.  %3d"     //Do not break the code!!
#define SERVO_LIVESETUP     "Livesetup %3s"     //Do not break the code!!
#define SERVO_PREVIOUS      "previous: %3d"     //Do not break the code!!
#define AUTO_AUTOSTART      "Autostart %3s"     //Do not break the code!!
#define AUTO_JARTOLERANCE   "Jar tol. %c%2dg"     //Do not break the code!!
#define AUTO_AUTOCORRECTION "Autocorr. %3s"     //Do not break the code!!
#define AUTO_OVERFILL       "Overfill %2dg"     //Do not break the code!!
#define PARAMETER_BUZZER    "Buzzer    %3s"     //Do not break the code!!
#define PARAMETER_MENU      "Menu   %6s"     //Do not break the code!!
#define PARAMETER_LIST      " List"
#define PARAMETER_SCROLL    "Scroll"
#define CLEARPREFS_ERASE    "Erase EEPROM"
#define COUNT_DISPLAY1      "%4dg %3s"     //Do not break the code!!
#define COUNT_DISPLAY2      "%5d Pcs."     //Do not break the code!!
#define COUNT_DISPLAY3      "Total trip:"
#define COUNT_DISPLAY4      "Total:"
#define JARTYPE_1           "TO42"
#define JARTYPE_2           "TO63"
#define JARTYPE_3           "TO82"

// Elements of the manual and automatic screen display
#define SCREEN_NOJAR1       "Put a jar"
#define SCREEN_NOJAR2       "on scale"
#define SCREEN_NOTARE       "no tare!"
#define SCREEN_SHORT1       "A=%-3d %2s %3d%%"     //Do not break the code!!
#define SCREEN_SHORT2       "AS"
#define SCREEN_SHORT3       "a"
#define SCREEN_SHORT4A      "c=   %s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4B      "c=   %s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT4C      "c=%-3d"     //Do not break the code!!
#define SCREEN_SHORT4D      "c=%-3d%s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4E      "c=%-3d%s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT5       "Manual  %s"     //Do not break the code!!
#define SCREEN_SHORT6       "Tare"
#define SCREEN_SHORT7       "A=%-3d    %3d%%"     //Do not break the code!!

// Error messages at startup
#define STARTUP_NOTCAL1     "Scale not"
#define STARTUP_NOTCAL2     "calibrated"
#define STARTUP_NOSCALE1    "No scale"
#define STARTUP_NOSCALE2    "detected"
#define STARTUP_WEIGHTON1   "Pls. empty"
#define STARTUP_WEIGHTON2   "the scale"

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
#define MENU_TARE           "Tares"
#define MENU_CALIBRATION    "Calibration"
#define MENU_CORRECTION     "Correction"
#define MENU_JARTYPES       "Contenants"
#define MENU_AUTO           "Automatique"
#define MENU_SERVO          "Servomoteur"
#define MENU_PARAMETERS     "Paramètres"
#define MENU_COUNT          "Compteur"
#define MENU_COUNTTRIP      "Compteur jour"
#define MENU_RESETPREFS     "Retour usine"

//  Various dialog messages used in different places
#define DIALOG_RESET        "Effacer"
#define DIALOG_CANCEL       "Annuler"
#define DIALOG_OK           "OK"
#define DIALOG_MISSING      " Manque"
#define DIALOG_SAVE         "Sauvegarder"
#define DIALOG_START        "START"
#define DIALOG_ON           "OUI"
#define DIALOG_OFF          "NON"

// Messages for various menu screens
#define CALIBRATION_PROMPT1 "Vider la"
#define CALIBRATION_PROMPT2 "balance"
#define CALIBRATION_PROMPT3 "et confirmer"
#define CALIBRATION_PROMPT4 "avec OK"
#define CALIBRATION_PROMPT5 "Mettre "
#define CALIBRATION_PROMPT6 "sur la balance"
#define CORRECTION_CORR     "Correction"
#define CORRECTION_PREVIOUS "valeur précéd."
#define SERVO_MINIMUM       "Minimum   %3d"     //Do not break the code!!
#define SERVO_MAXIMUM       "Maximum   %3d"     //Do not break the code!!
#define SERVO_FINE          "Dosage fin %3d"     //Do not break the code!!
#define SERVO_LIVESETUP     "Livesetup %3s"     //Do not break the code!!
#define SERVO_PREVIOUS      "précédent %3d"     //Do not break the code!!
#define AUTO_AUTOSTART      "Autostart %3s"     //Do not break the code!!
#define AUTO_JARTOLERANCE   "Tol. pot %c%2dg"     //Do not break the code!!
#define AUTO_AUTOCORRECTION "Autocorr. %3s"     //Do not break the code!!
#define AUTO_OVERFILL       "Surrempl. %2dg"     //Do not break the code!!
#define PARAMETER_BUZZER    "Buzzer    %3s"     //Do not break the code!!
#define PARAMETER_MENU      "Menu   %6s"     //Do not break the code!!
#define PARAMETER_LIST      "Liste"
#define PARAMETER_SCROLL    "Déroulant"
#define CLEARPREFS_ERASE    "Effacer Préfér."
#define COUNT_DISPLAY1      "%4dg %3s"     //Do not break the code!!
#define COUNT_DISPLAY2      "%5d Pcs."     //Do not break the code!!
#define COUNT_DISPLAY3      "Total jour:"
#define COUNT_DISPLAY4      "Total:"
#define JARTYPE_1           "TO42"
#define JARTYPE_2           "TO63"
#define JARTYPE_3           "TO82"

// Elements of the manual and automatic screen display
#define SCREEN_NOJAR1       "Placer un"
#define SCREEN_NOJAR2       "contenant"
#define SCREEN_NOTARE       " tare!  "
#define SCREEN_SHORT1       "A=%-3d %2s %3d%%"     //Do not break the code!!
#define SCREEN_SHORT2       "AS"
#define SCREEN_SHORT3       "a"
#define SCREEN_SHORT4A      "c=   %s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4B      "c=   %s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT4C      "c=%-3d"     //Do not break the code!!
#define SCREEN_SHORT4D      "c=%-3d%s %3s-%3s"     //Do not break the code!!
#define SCREEN_SHORT4E      "c=%-3d%s %3d-%3s"     //Do not break the code!!
#define SCREEN_SHORT5       "Manuel  %s"     //Do not break the code!!
#define SCREEN_SHORT6       "Tare"
#define SCREEN_SHORT7       "A=%-3d    %3d%%"     //Do not break the code!!

// Error messages at startup
#define STARTUP_NOTCAL1     "Balance non"
#define STARTUP_NOTCAL2     "calibrée"
#define STARTUP_NOSCALE1    "Pas de balance"
#define STARTUP_NOSCALE2    "detectée"
#define STARTUP_WEIGHTON1   "Vider la"
#define STARTUP_WEIGHTON2   "balance"

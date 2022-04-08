


void Task0_rotary(void * parameter) {
  static int aState;
  static int aLastState = 2;  // reale Werte sind 0 und 1
  for(;;) {
  delay(1);
  
  //Serial.println(".");
  //if ( rotating ) delay (50);  // wait a little until the bouncing is done
   
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
      Serial.print(" Rotary Value changed to ");
      Serial.println(getRotariesValue(rotary_select));
#endif 
#endif 
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
}
}


//////////////////////// Fun ;-) /////////////////////////////////////


const int c = 261;
const int d = 294;
const int e = 329;
const int f = 349;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 455;
const int b = 466;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659; 
const int fH = 698;
const int fSH = 740;
const int gH = 784;
const int gSH = 830;
const int aH = 880;
 
const int buzzerPin = 25;
 
void beep(int note, int duration)
{
  tone(buzzerPin, note, duration);
  noTone(buzzerPin); 
  delay(50);
}
 
void starwarsTheme()
{
  beep(a, 500);
  beep(a, 500);    
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);  
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
 
  delay(500);
 
  beep(eH, 500);
  beep(eH, 500);
  beep(eH, 500);  
  beep(fH, 350);
  beep(cH, 150);
  beep(gS, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
 
  delay(500);
}
 

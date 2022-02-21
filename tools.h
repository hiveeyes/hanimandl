


void Task0_rotary(void * parameter) {
  static int aState;
  static int aLastState = 2;  // reale Werte sind 0 und 1
  for(;;) {
  delay(5);
  
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



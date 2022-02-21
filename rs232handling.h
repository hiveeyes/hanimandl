
//MarcN: Thread auf Kern0. rs232 Waage
void Task0_rs232reader( void * parameter) { 
  long unsigned lastFlowTimestamp;  
  unsigned long timetmp;
  int lastFlowWeight;
  int flowPeriode;
  lastFlowTimestamp = millis();
  lastFlowWeight = 0;
  flowPeriode = 1000;
  char serial_peek;
  char tmpChar;
 for(;;) {
    delay(rs232wait);  // kurz warten, um Nachzügler beim löschen zu erwischen
    Serial2.readBytes(serbuf, Serial2.available());         // Puffer komplett leeren
    Serial2.write(rs232request,sizeof(rs232request));
    timetmp = millis();
    Serial2.flush();         // Warte, bis das Senden abgeschlossen ist
    while ( ( Serial2.available() <= 8 ) && ( ( millis() - timetmp ) < rs232Timeout )) {   
      delay(1); // Es wurden noch zu wenig Daten von der Waage gesendet. Abwarten.. bis timeout
    } 
    char serial_peek = Serial2.peek();
    while( (Serial2.available() > 0) && (serial_peek != '+') && (serial_peek != '-') ) {   // Alle Zeichen vor einem + oder - abschneiden
        // Ein korrektes Paket fängt mit + oder - an. Schneide alles andere ab
        //Serial.print("Kein +- am Anfang: <"); Serial.print((byte)serial_peek);Serial.println(">");
        tmpChar = Serial2.read();   // das erste Zeichen im Puffer löschen
        Serial.print("cutting non + - : <");Serial.print((byte)tmpChar);Serial.println(">");
        serial_peek = Serial2.peek();      
    }    
    serlen = Serial2.available();    
    if ( (serlen < 6) || (serlen == 0) ) {  // Ist nach dem Trimmen noch ein Paket mit min 6 Zeichen in Puffer ? 
       // Kein vollständiger Datensatz:
       //Serial.println("Weniger als 6 Zeichen im Puffer! Möglicherweise falsches Ergebnis.");  
       //Serial2.readBytes(serbuf, Serial2.available());   // Puffer leeren
    }
    else {     // alles OK ! Das Paket fängt mit + oder - an und es folgen mindestens 5 weitere Zeichen
       Serial2.readBytes(serbuf, Serial2.available());  // den gesamten rs232 Puffer laden und damit löschen
       rs232weight.weight = (atof( &(serbuf[1]) ));   // ab dem zweiten Zeichen steht das Gewicht 
       if ( serbuf[0] == '-' ) {                      // liegt ein negatives Gewicht im Array ? 
          rs232weight.weight *= -1;
       }
       rs232weight.delta = millis() - rs232weight.timestamp;
       rs232weight.timestamp = millis();
   }
 }
}


int getWeight(int n) {
   //return 0;
  
  if ( (millis() - rs232weight.timestamp) < maxWeightAge) {
    return rs232weight.weight;
  }
  else {
    // Letzte Gewichtsmessung zu alt. Waage im Standby oder Kabel ab !
    
#ifdef isDebug
    Serial.print("rs232 Paket zu alt ->  ");
    Serial.println(millis() - rs232weight.timestamp);
#endif 
return -999;
  }
  
}

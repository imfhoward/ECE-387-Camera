void longPress(){

for (int j = 0; j < 10; j++) {//flash the LED
    digitalWrite(LEDpin, !digitalRead(LEDpin));
    delay(200);
  }
  digitalWrite(LEDpin, LOW);
  unsigned int blinkCounter = 0;
  //readFile();
  //WiFiConnection();//get connected to the WiFi
  //delay(1000);
  startOTA();//start up OTA
  for (unsigned int i = 0; i < 120000; i++) {//2 min to update or external trigger will reset board
    ArduinoOTA.handle();//check for OTA
    blinkCounter++;
    if (blinkCounter > 1000) {//slow flash of LED
      digitalWrite(LEDpin, !digitalRead(LEDpin));
      blinkCounter = 0;
    }

    delay(1);//using a 1ms increment

    //if you trigger the board externally now, you can reset the WiFi Settings
    if (digitalRead(EXTWAKEpin) == LOW) {  //HIGH if V2, LOW if V3
      for (int j = 0; j < 10; j++) {
        digitalWrite(LEDpin, !digitalRead(LEDpin));
        delay(100);
      }
      digitalWrite(LEDpin, LOW);
      resetFlag = true;
      WiFiConnection();
      break;
    }
  }
  
}


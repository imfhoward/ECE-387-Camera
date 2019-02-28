void checkTrigger() {
  pinMode(EXTWAKEpin, INPUT);// need to immediately check this pin

  //HIGH if V2, LOW if V3
  if (digitalRead(EXTWAKEpin) == LOW) {//flag if HIGH, then the external wake circuit woke the board up
    externalWake = true;
  }

  pinMode(DONEpin, OUTPUT);

  batteryVoltage = 0.00478 * analogRead(A0) - 0.015;//scale that 1V max to 0-4.5V
  
  pinMode(LEDpin, OUTPUT);// LED on Board
  digitalWrite(LEDpin, LOW); //LOW to turn on

  //pinMode(DONEpin, OUTPUT);//when this goes HIGH, the board will sleep

  Serial.println("");
  if (externalWake)
    Serial.println("external wake");
  else {
    Serial.println("timer wake");
    //if (batteryVoltage >= 3.3)
    //digitalWrite(DONEpin, HIGH);// standard wake, so just go back to sleep, who cares...
  }
  Serial.print("Battery Voltage = ");
  Serial.println(batteryVoltage);
  //delay(500);
  //digitalWrite(DONEpin, HIGH);
  dtostrf(batteryVoltage, 3, 1, batteryBuf);//converts battery voltage float to char array

}


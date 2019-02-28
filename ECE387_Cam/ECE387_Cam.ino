#include "Wire.h"
#include <ArduCAM.h>// Arducam library //https://github.com/ArduCAM/Arduino
#include <SPI.h>
#include "memorysaver.h"
#include <time.h>
#include <SoftwareSerial.h>
#include <SD.h>


const int CS = 7, picCnt = 0;
SD sd;
SD myFile;

ArduCAM myCAM( OV2640, CS );

#define DONEpin 4
#define EXTWAKEpin 2 

void takePicture();
void uploadPicture();


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
for (int i = 0; i < 10; i++) {
    digitalWrite(DONEpin, HIGH);
    delay(100);
    digitalWrite(DONEpin, LOW);
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DONEpin, HIGH);
  delay(5000);
  digitalWrite(DONEpin, LOW);
  delay(5000);
}

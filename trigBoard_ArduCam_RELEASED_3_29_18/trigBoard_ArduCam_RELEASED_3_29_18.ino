//trigBoard Sample Code with Arducam Uploading to server and posting to slack
// 3/29/17  Kevin Darrah
// RELEASED

// Includes - may not all be needed
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "Wire.h"
#include <ArduCAM.h>// Arducam library //https://github.com/ArduCAM/Arduino
#include <SPI.h>
#include "memorysaver.h"
#include <time.h>

//camera variables
const int CS = 16;// NOTE THIS IS SHARED WITH EXT WAKE PIN!!!

ArduCAM myCAM( OV2640, CS );//setup for ArduCam mini 2MP


// PINS!!!
#define LEDpin 0
#define DONEpin 15  
#define EXTWAKEpin 16 

//FUNCTIONS
void PushStatus(const char *MessageBody, const char *MessageTitle, const char *MessageType, ...);// to push messages
void PushSlack(const char *MessageBody, const char *MessageTitle, const char *MessageType, ...);//for slack messages


void saveConfigCallback ();
void readFile();
boolean WiFiConnection();
void checkTrigger();
void startOTA();
void longPress();

//camera functions
void takePicture();
void uploadPicture();

char filename[30];//used for the name we save the picture as

//globals for credentials
char tokenKey[100];
char trigBoardName[40];
char triggerMessage[100];
char fileURL[100];//where we send the image - new for the Arducam version

boolean resetFlag = false;

//flag for saving data
bool shouldSaveConfig = false;

char batteryBuf[5] = "";// used to convert float to a char array

boolean externalWake = false;// to figure out who woke up the board
float batteryVoltage = 0;
float batteryThreshold = 3.4;

void setup() {

  Serial.begin(115200);//debug
  checkTrigger();
  if (externalWake || batteryVoltage < batteryThreshold) {// only if external trigger was fired or the battery is low, send a push
    takePicture();//take the picture
    readFile();//go get all of the credentials and stuff needed for the connection
    WiFiConnection();//connect and send data (in provisioning tab)
  }
  for (int i = 0; i < 10; i++) {
    digitalWrite(DONEpin, HIGH);
    delay(100);
    digitalWrite(DONEpin, LOW);
    delay(100);
  }
  longPress();

}

void loop() {
  digitalWrite(DONEpin, HIGH);
  delay(5000);
  digitalWrite(DONEpin, LOW);
  delay(5000);
}



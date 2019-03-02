#include <SoftwareSerial.h>
#include <SdFat.h>

//SD Card
SdFat sd;
SdFile myFile;
int picCnt = 0, door = 7, alarm = 8;

//Camera
byte incomingbyte;
SoftwareSerial cameraSerial = SoftwareSerial(2, 3);   //Configure pin 2 and 3 as soft serial port
int a = 0x0000, j = 0, k = 0, count = 0; //Read Starting address
uint8_t MH, ML;
boolean EndFlag = 0;

//Declare pins
const int chipSelect = 10;

void setup() {
  pinMode(door, INPUT);
  pinMode(alarm, OUTPUT);
  Serial.begin(19200); //start serial
  cameraSerial.begin(38400); //start serial with camera
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  SendResetCmd(); //allows camera to take pictures
  delay(3000); //delay necessary for camera reset
}

void loop() {
  //create title for images
  char photoTitle[25] = {};
  sprintf(photoTitle, "pic%d.txt", picCnt);

  //make sure file can be created, otherwise print error
  if (!myFile.open(photoTitle, O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening photoTitle.txt for write failed");
  }

  CheckAlarm();

  while (cameraSerial.available() > 0) {
    incomingbyte = cameraSerial.read(); //clear unneccessary serial from camera
  }
  byte b[32];

  while (!EndFlag) {
    j = 0;
    k = 0;
    count = 0;
    SendReadDataCmd(); //command to get picture from camera

    delay(75); //delay necessary for data not to be lost
    while (cameraSerial.available() > 0) {
      incomingbyte = cameraSerial.read(); //read serial from camera
      k++;
      if ((k > 5) && (j < 32) && (!EndFlag)) {
        b[j] = incomingbyte;
        if ((b[j - 1] == 0xFF) && (b[j] == 0xD9))
          EndFlag = 1; //when end of picture appears, stop reading data
        j++;
        count++;
      }
    }

    for (j = 0; j < count; j++) { //store picture into file
      if (b[j] < 0x10)
        myFile.print("0");
      myFile.print(b[j], HEX);
    }
    myFile.println();
  }

  StopTakePhotoCmd(); //stop this picture so another one can be taken
  EndFlag = 0; // reset flag to allow another picture to be read
  Serial.println("End of pic");
  Serial.println();
  myFile.close(); //close file

  picCnt++; //increment value for next picture

}

void CheckAlarm() {
  if (digitalRead(door) == HIGH) {
    tone(alarm, 2000);
    SendTakePhotoCmd(); //take photo
    delay(200);//delay to make sure there is no drop in the data
  }
  else {
    delay(200);
    CheckAlarm();
  }
}

//Send Reset command
void SendResetCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x26);
  cameraSerial.write((byte)0x00);
}

//Send take picture command
void SendTakePhotoCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x36);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x00);

  a = 0x0000; //reset so that another picture can taken
}

void FrameSize() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x34);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x00);
}

//Read data
void SendReadDataCmd() {
  MH = a / 0x100;
  ML = a % 0x100;

  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x32);
  cameraSerial.write((byte)0x0c);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x0a);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)MH);
  cameraSerial.write((byte)ML);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x20);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x0a);

  a += 0x20;
}

void StopTakePhotoCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x36);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x03);
}

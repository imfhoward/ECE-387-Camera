void takePicture() {
  uint8_t vid, pid;
  uint8_t temp;
  delay(100);

  while (digitalRead(EXTWAKEpin) == LOW) {
    yield(); // wait for the external pin to go LOW - could still be HIGH, so gotta wait
  }
  pinMode(CS, OUTPUT);//now we can use the CS pin for the camera
  SPI.begin();//uses SPI
  Wire.begin();//also I2C
  while (1) {//while loop a little dangerous, but this makes sure the camera gets initilized
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("ACK CMD SPI interface Error!"));
      delay(10);
      continue;
    } else {
      Serial.println(F("ACK CMD SPI interface OK.")); break;//got what we needed, camera is good, break out of while
    }
  }

  while (1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
      Serial.println(F("ACK CMD Can't find OV2640 module!"));
      //delay(1000);
      continue;
    }
    else {
      Serial.println(F("ACK CMD OV2640 detected.")); break;
    }
  }

// ALL GOOD - init the camera
  myCAM.set_format(JPEG);
  myCAM.InitCAM();

  // THIS SETS THE RESOLUTION!!!
  //myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  //myCAM.OV2640_set_JPEG_size(OV2640_640x480);
  myCAM.OV2640_set_JPEG_size(OV2640_800x600);// This seems to be the max this camera can take
  //myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
  delay(1100);//have to take a bit of time to let the config go through
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();//take the picture!!
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("ACK CMD CAM Capture Done."));//DONE
  pinMode(CS, INPUT);//NOW we can release the CS pin for a bit until we need to read out the picture and upload
}



void uploadPicture() {
  //create title for images 
  char photoTitle[25] = {}; 
  sprintf(photoTitle, "pic%d.txt", picCnt); //make sure file can be created, otherwise print error
  if (!myFile.open(photoTitle, O_RDWR | O_CREAT | O_AT_END)) {
      sd.errorHalt("opening photoTitle.txt for write failed");
    }
}

void SendReadDataCmd() {
  MH=a/0x100;
  ML=a%0x100;
      
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

  a+=0x20; 
}

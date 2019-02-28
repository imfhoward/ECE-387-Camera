void takePicture() {

  // this code was lifted out of the example code for the ArduCam
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
  digitalWrite(LEDpin, HIGH); //LOW to turn on
  pinMode(CS, INPUT);//NOW we can release the CS pin for a bit until we need to read out the picture and upload
}


void uploadPicture() {// this uploads the image to the server
  uint8_t temp = 0xff;
  uint8_t temp_last = 0;
  bool is_header = false;
  uint32_t length = 0;

  pinMode(CS, OUTPUT);//can use the CS pin again for reading from the camera

  length = myCAM.read_fifo_length();//get the length of data we need to read out
  Serial.println(length, DEC);
  int i = 0, j = 0;//amazing naming of variables!

  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");//this gets a time stamp
  while (!time(nullptr)) {//give it some time to get the time from the server
    Serial.print(".");
    delay(100);
  }
  time_t now = time(nullptr);//full time stamp
  //Serial.print(ctime(&now));

//now that we have the timestamp, can build up the filename
  String timeName = "";//used for the filename
  timeName.concat(ctime(&now));//first the timestamp
  timeName.trim();//get the stuff we don't need off it
  for (int i = 0; i < timeName.length(); i++) {//this strips the spaces out replaces with _
    if (timeName[i] == ' ')
      timeName[i] = '_';
  }
  timeName.concat(".jpg");//add the extension, and we're done!


  Serial.println(timeName);
  timeName.toCharArray(filename, timeName.length() + 1);//create the filename... just converts the string to array
  String payload = "value=";//what is sent in the post to the server

  HTTPClient http;
  String targetURL = String(fileURL) + "frommicro.php?data=";
  
  targetURL.concat(timeName);//add the filename to the url
  Serial.println(targetURL);
  int httpCode;//getting ready for the connection

  myCAM.CS_LOW();//let's start reading the data!
  myCAM.set_fifo_burst();//Set fifo burst mode
  temp =  SPI.transfer(0x00);//get the first byte, and store in temp
  length --;//take a byte off length, since we read one out
  while ( length-- )//loop through the entire buffer
  {
    temp_last = temp;//this is why we already read a byte out, we can store in as the last byte read
    temp =  SPI.transfer(0x00);//read a byte
    if (is_header == true)//if the header is found
    {

      if (payload.length() < 1500) {//we don't want to send teh entire packet at once, so this sets the packet size
        payload.concat(byte(temp));//add a byte
        payload.concat(",");//need a comma separator
      }
      else {
        // TIME TO CONNECT AND SEND WHAT WE HAVE SO FAR
        digitalWrite(LEDpin, LOW); //LOW to turn on - flash the LED
        http.begin(targetURL); //HTTP connection to the php file
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpCode = http.POST(payload);//send the PAYLOAD
        Serial.println(httpCode);
        Serial.println("File in Cloud is ");// this tells you how many bytes were sent
        http.writeToStream(&Serial);
        Serial.println(" bytes");
        http.end();//all done
        payload = "value=";//reset the payload string
        payload.concat(byte(temp));//we had a byte already, add it in
        payload.concat(",");//don't forget the comma
        digitalWrite(LEDpin, HIGH); //LOW to turn on
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))// this is why we need to keep track of the last byte, so we can see the order of the two bytes
    {// 0XFF then 0XDD is the header
      is_header = true;//header found!
      Serial.println(F("ACK IMG"));
      Serial.write(temp_last);
      Serial.write(temp);
      payload.concat(byte(temp_last));//add to the payload
      payload.concat(",");//everything separated with commas
      payload.concat(byte(temp));//next byte
      payload.concat(",");//comma again
    }
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) // 0xFF then 0xD9 is end
      break;
    yield();//added for the ESP8266
  }

//done, but what if we hit the end of the buffer before reaching the data packet size?
//this takes care of that and sends the last bit out
  if (payload.length() > 1) {//check the payload if there's still something in there... might be a bug here, but there will always be stuff here
    http.begin(targetURL); //HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpCode = http.POST(payload);//same as before, just send the payload out
    Serial.println(httpCode);
    Serial.println("File in Cloud is ");
    http.writeToStream(&Serial);
    Serial.println(" bytes");
    http.end();
    payload = "";
  }
  pinMode(CS, INPUT);// THIS IS KEY - we need to release the CS pin (EXT WAKE) so now you can achieve the other modes - OTA and WiFi Reset
}


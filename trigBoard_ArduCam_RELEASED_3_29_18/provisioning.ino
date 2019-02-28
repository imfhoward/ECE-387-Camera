void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

boolean WiFiConnection() {

  WiFiManager wifiManager;

  if (resetFlag)
    wifiManager.resetSettings();// held the button long enough, so wipe out the wifi settings and launch the portal

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter customTokenKey("Token", "0.xxxx Token From PushBullet/ or SLACK URL", tokenKey, 100);
  WiFiManagerParameter customtrigBoardName("trigBoardName", "Unique Name", trigBoardName, 40);
  WiFiManagerParameter customTriggerMessage("TriggerMessage", "Trigger Message to be Sent", triggerMessage, 100);
  WiFiManagerParameter customfileURL("fileURL", "url where image is sent - http://yourURL.com/place/", fileURL, 100);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&customTokenKey);
  wifiManager.addParameter(&customtrigBoardName);
  wifiManager.addParameter(&customTriggerMessage);
  wifiManager.addParameter(&customfileURL);

  //reset settings - for testing
  //wifiManager.resetSettings();
  wifiManager.setTimeout(120);//2min, if can't get connected just go back to sleep
  if (!wifiManager.autoConnect("trigBoard")) {// SSID you connect to from browser
    Serial.println("failed to connect and hit timeout");
    digitalWrite(DONEpin, HIGH);
    return 0;
  }

  Serial.println("Connected");

  //read updated parameters
  strcpy(tokenKey, customTokenKey.getValue());
  strcpy(trigBoardName, customtrigBoardName.getValue());
  strcpy(triggerMessage, customTriggerMessage.getValue());
  strcpy(fileURL, customfileURL.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["tokenKey"] = tokenKey;
    json["trigBoardName"] = trigBoardName;
    json["triggerMessage"] = triggerMessage;
    json["fileURL"] = fileURL;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("");// WiFi info
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());





  char pushMessage[100];

  if (batteryVoltage >= batteryThreshold) {
    uploadPicture();// if all is good, upload the picture
    sprintf(pushMessage, "Battery=%sV  %simages/%s", batteryBuf,fileURL, filename);
    PushSlack(pushMessage, trigBoardName, "note");//sending this to slack
  }
  else {
    sprintf(pushMessage, "%s-BATTERY LOW! Battery=%sV", trigBoardName, batteryBuf);
    PushSlack(pushMessage, "BATTERY LOW", "note");
  }
  //String pushMessage = trigBoardName + triggerMessage;

}


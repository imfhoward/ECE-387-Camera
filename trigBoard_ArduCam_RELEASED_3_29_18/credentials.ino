void readFile() {
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");
  //SPIFFS.format();
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          if (json.containsKey("triggerMessage") && json.containsKey("trigBoardName") && json.containsKey("tokenKey") && json.containsKey("fileURL")){
            strcpy(triggerMessage, json["triggerMessage"]);
            strcpy(trigBoardName, json["trigBoardName"]);
            strcpy(tokenKey, json["tokenKey"]);
            strcpy(fileURL, json["fileURL"]);
            Serial.println("\nkeys found - all good");
          }
          else {
            SPIFFS.format();
            digitalWrite(DONEpin, HIGH);
          }

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



}


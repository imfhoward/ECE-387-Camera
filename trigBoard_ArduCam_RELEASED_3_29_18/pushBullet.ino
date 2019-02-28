// Richard Johnson wrote this function for me: https://www.youtube.com/channel/UCLgqthbPlZGr8DuHDKKr9EA
// The Pushbullet HTTP post code came from here: https://steve0hh.github.io/2016/12/04/how-to-send-push-notifications-using-esp8266.html




void PushStatus(const char *MessageBody, const char *MessageTitle, const char *MessageType, ...) {
  int PRINTF_BUF = 100;
  int StartWordCount = 32;
  const char* host = "api.pushbullet.com";
  //https://api.pushbullet.com/v2/pushes
  const int httpsPort = 443;

  char k = 0;
  char buf[PRINTF_BUF];
  va_list args;
  va_start(args, MessageBody);
  vsnprintf(buf, sizeof(buf), MessageBody, args);
  while (MessageType[k] != '\0') {
    k++;
  }
  int CharCount = StartWordCount + k;
  k = 0;
  while (MessageTitle[k] != '\0') {
    k++;
  }
  CharCount = CharCount + k;
  k = 0;
  while (buf[k] != '\0') {
    k++;
  }
  CharCount = CharCount + k;


  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
    // digitalWrite(DONEpin, HIGH);
  }
  String url = "/v2/pushes";

  va_end(args);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Access-Token: " + tokenKey + "\r\n" +
               "Content-length: " + CharCount + "\r\n"
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n" +
               "{\"body\":\"" + buf + "\",\"title\":\"" + MessageTitle + "\",\"type\":\"note\"}"
              );

}

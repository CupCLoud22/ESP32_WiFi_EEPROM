// ESP32 HTTP webserver: receiving textual data in websocket
// send text from Python Code to ESP32

#include <WiFi.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
 
//const char* ssid = "RAK7258_3DB3";
//const char* password =  "kebunbapak123";

#define LENGTH(x) (strlen(x) + 1)   // length of char string
#define EEPROM_SIZE 200             // EEPROM size
#define WiFi_rst 0                  //WiFi credential reset pin (Boot button on ESP32)
String ssid;                        //string variable to store ssid
String pss;                         //string variable to store password
unsigned long rst_millis;

String deviceName, provisionDeviceKey, provisionDeviceSecret;
 
AsyncWebServer server(80);
AsyncWebSocket ws("/test");
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){

    /*
    char JSONMessage[] = " {\"deviceName\": \"DEVICE_NAME\", \"provisionDeviceKey\": \"PUT_PROVISION_KEY_HERE\", \"provisionDeviceSecret\": \"PUT_PROVISION_SECRET_HERE\"}"; //Original message

    StaticJsonBuffer<300> JSONBuffer;
    JsonObject& parsed = JSONBuffer.parseObject(JSONMessage); //Parse message

    if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(5000);
    return;
 
    }

    const char * deviceName = parsed["deviceName"];
    const char * provisionDeviceKey = parsed["provisionDeviceKey"];
    const char * provisionDeviceSecret = parsed["provisionDeviceSecret"];
    
    //Serial.print("Websocket client connection received"); // seroal monitor
    Serial.print("deviceName: ");
    Serial.println(deviceName);
    Serial.print("provisionDeviceKey: ");
    Serial.println(provisionDeviceKey);
    Serial.print("provisionDeviceSecret: ");
    Serial.println(provisionDeviceSecret);

    */

    deviceName = "DEVICE_NAME";
    provisionDeviceKey = "PUT_PROVISION_KEY_HERE";
    provisionDeviceSecret = "PUT_PROVISION_SECRET_HERE";

    JSONVar doc;
    doc["deviceName"] = String(deviceName);
    doc["provisionDeviceKey"] = String(provisionDeviceKey);
    doc["provisionDeviceSecret"] = String(provisionDeviceSecret);
    String tmp = JSON.stringify(doc);
    //Serial.print();
    Serial.println(tmp);
    
    client->text(tmp); // terhubung ke python
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected"); // serial monitor
 
  } else if(type == WS_EVT_DATA){
 
    Serial.println("Data received: "); //serial monitor
 
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
 
    Serial.println();
  }
}
 
void setup(){
  Serial.begin(115200);
  /*
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
  Serial.println("Connected.");
  Serial.println();
 
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();
  */

  
  pinMode(WiFi_rst, INPUT);
  if (!EEPROM.begin(EEPROM_SIZE)) { //Init EEPROM
    Serial.println("failed to init EEPROM");
    delay(1000);
  }
  else
  {
    ssid = readStringFromFlash(0); // Read SSID stored at address 0
    Serial.print("SSID = ");
    Serial.println(ssid);
    pss = readStringFromFlash(40); // Read Password stored at address 40
    Serial.print("PSS = ");
    Serial.println(pss);
  }

  WiFi.begin(ssid.c_str(), pss.c_str());

  delay(3500);   // Wait for a while till ESP connects to WiFi

  if (WiFi.status() != WL_CONNECTED) // if WiFi is not connected
  {
    //Init WiFi as Station, start SmartConfig
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("SmartConfig received.");

    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi Connected.");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // read the connected WiFi SSID and password
    ssid = WiFi.SSID();
    pss = WiFi.psk();
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("PSS:");
    Serial.println(pss);
    Serial.println("Store SSID & PSS in Flash");
    writeStringToFlash(ssid.c_str(), 0); // storing ssid at address 0
    writeStringToFlash(pss.c_str(), 40); // storing pss at address 40
  }
  else
  {
    Serial.println("WiFi Connected");
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  rst_millis = millis();
  while (digitalRead(WiFi_rst) == LOW)
  {
    // Wait till boot button is pressed 
  }
  // check the button press time if it is greater than 3sec clear wifi cred and restart ESP 
  if (millis() - rst_millis >= 3000)
  {
    Serial.println("Reseting the WiFi credentials");
    writeStringToFlash("", 0); // Reset the SSID
    writeStringToFlash("", 40); // Reset the Password
    Serial.println("Wifi credentials erased");
    Serial.println("Restarting the ESP");
    delay(500);
    ESP.restart();            // Restart ESP
  }

}

void writeStringToFlash(const char* toStore, int startAddr) {
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}


String readStringFromFlash(int startAddr) {
  char in[128]; // char array of size 128 for reading the stored data 
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}

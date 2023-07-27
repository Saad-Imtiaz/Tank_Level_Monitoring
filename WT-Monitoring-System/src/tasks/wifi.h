#ifndef WIFI
#define WIFI

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h> 
#include <AsyncWebSocket.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws; 


// Search for parameter in HTTP POST request
extern const char* PARAM_INPUT_2;
extern const char* PARAM_INPUT_1;
extern const char* PARAM_INPUT_3;
extern const char* PARAM_INPUT_4;

extern String ssid;
extern String pass;
extern String ip;
extern String gateway;

// File paths to save input values permanently
extern const char* ssidPath;
extern const char* passPath;
extern const char* ipPath;
extern const char* gatewayPath;


extern IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded
// Set your Gateway IP address
extern IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
extern IPAddress subnet;

// Timer variables
extern unsigned long previousMillis;
extern const long interval; 


extern float  totalDepth; // Total depth of the tank in m
extern float  tankLength; // Length of the tank in m
extern float  tankWidth; // Width of the tank in m

extern float tankVolume; // Volume of the tank in liters
extern float remainingWater; // Remaining liters in the tank
extern float tankLiters; // Total liters in the tank
extern float distance_cm;


void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { }

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    #if DEBUG == true
    Serial.println("An error has occurred while mounting SPIFFS");
    #endif
  }
  #if DEBUG == true
  Serial.println("SPIFFS mounted successfully");
    #endif
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
    #if DEBUG == true
  Serial.printf("Reading file: %s\r\n", path);
#endif
  File file = fs.open(path);
  if(!file || file.isDirectory()){
    #if DEBUG == true
    Serial.println("- failed to open file for reading");
    #endif
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
    #if DEBUG == true
  Serial.printf("Writing file: %s\r\n", path);
#endif
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    #if DEBUG == true
    Serial.println("- failed to open file for writing");
    #endif
    return;
  }
  if(file.print(message)){
    #if DEBUG == true
    Serial.println("- file written");
    #endif
  } else {
    #if DEBUG == true
    Serial.println("- write failed");
    #endif
  }
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    #if DEBUG == true
    Serial.println("Undefined SSID or IP address.");
    #endif
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)){
    #if DEBUG == true
    Serial.println("STA Failed to configure");
    #endif
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
    #if DEBUG == true
    Serial.println("Connecting to WiFi...");
    #endif
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    #if DEBUG == true
      Serial.println("Failed to connect.");
    #endif
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

void readWiFiSettings(){
 // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile (SPIFFS, gatewayPath);
 #if DEBUG == true
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
#endif
}

void handleSettings(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String totalDepthStr = request->getParam("totalDepth")->value();
    String tankLengthStr = request->getParam("tankLength")->value();
    String tankWidthStr = request->getParam("tankWidth")->value();

    totalDepth = totalDepthStr.toFloat();
    tankLength = tankLengthStr.toFloat();
    tankWidth = tankWidthStr.toFloat();

    // Save the parameters to SPIFFS as .json file
    DynamicJsonDocument jsonDoc(256); // Increase buffer size for JSON document
    jsonDoc["totalDepth"] = totalDepth;
    jsonDoc["tankLength"] = tankLength;
    jsonDoc["tankWidth"] = tankWidth;

    File configFile = SPIFFS.open("/config.json", FILE_WRITE);
    if (!configFile) {
        #if DEBUG == true
        Serial.println("Failed to open config file for writing");
        #endif
      return;
    }

    if (serializeJson(jsonDoc, configFile) == 0) {
      configFile.close();
      #if DEBUG == true
       Serial.println("256, text/plain, Failed to write JSON data to config file.");
        #endif
      return;
    }

    configFile.close();
    #if DEBUG == true
   Serial.println("256,text/plain, Settings saved.");
    #endif
    request->send(200, "text/plain", "Settings saved.");
  }
  
   else if (request->method() == HTTP_GET) {
    // Read the saved parameters from SPIFFS .json file
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      DynamicJsonDocument jsonDoc(256); // Increase buffer size for JSON document

      DeserializationError error = deserializeJson(jsonDoc, configFile);
      configFile.close();

      if (error) {
        request->send(500, "text/plain", "Error reading settings.");
        return;
      }

      JsonObject jsonObject = jsonDoc.as<JsonObject>();
      String response;
      serializeJson(jsonObject, response);

      request->send(200, "application/json", response);
    } else {
      request->send(404, "text/plain", "Settings not found.");
    }
  }
}
void runWiFi(){

    if(initWiFi()) {
    // Route for root / page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false); // Specify the content type
  });

    // Route for settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html");
  });

  // Route to handle settings form submission
  server.on("/save-settings", HTTP_POST, handleSettings);
    // Add a new route to handle the reset request
  
  // Route to get saved settings
  server.on("/get-settings", HTTP_GET, handleSettings);
  
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Perform the reset here
    ESP.restart();
  });

  // Serve static files (CSS, JS, images, etc.)
  server.serveStatic("/", SPIFFS, "/");
 server.begin();
  }
  else {
    #if DEBUG == true
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    #endif
    // NULL sets an open Access Point
    WiFi.softAP("AdlerOne-AP", NULL);

    IPAddress IP = WiFi.softAPIP();

    #if DEBUG == true
    Serial.print("AP IP address: ");
    Serial.println(IP); 
    #endif
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            #if DEBUG == true
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            #endif
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            #if DEBUG == true
            Serial.print("Password set to: ");
            Serial.println(pass);
            #endif
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            #if DEBUG == true
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            #endif
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            #if DEBUG == true
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            #endif
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
}
}
#endif
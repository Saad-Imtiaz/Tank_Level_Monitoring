#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h> 
#include <AsyncWebSocket.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "tasks/sensors.h"
#include "tasks/wifi.h"


AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // Create a WebSocket instance, you can change the URL path if needed.

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

float  totalDepth = TOTAL_DEPTH; 
float  tankLength = TANK_LENGHT; 
float  tankWidth = TANK_WIDTH; 

float tankVolume = 0; // Volume of the tank in liters
float remainingWater = 0; // Remaining liters in the tank
float tankLiters = 0; // Total liters in the tank
float distance_cm;

void RESET_TIMER()
{
  if (millis() > RESTART_TIME)
  {
    ESP.restart();
  }
}
void setup() {

  #if DEBUG == true
  Serial.begin(115200);
  #endif

  setupSensors();
  initSPIFFS();
  readWiFiSettings();
  
  #if DEBUG == true
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  tankVolume = calculateTankVolume(totalDepth, tankLength, tankWidth);
  tankLiters = tankVolume * 1000;
  #endif

  runWiFi();
 
  // Route for root / web page  
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  Serial.println("Web server started!");


}

void loop() {
  RESET_TIMER();
  updateWaterLevel();
}
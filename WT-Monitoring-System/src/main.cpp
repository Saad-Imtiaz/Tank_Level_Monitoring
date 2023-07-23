#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h> 
#include <AsyncWebSocket.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>


AsyncWebSocket ws("/ws"); // Create a WebSocket instance, you can change the URL path if needed.

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Not used for this example, but you can add custom WebSocket event handling here if needed.
}
int trigger_pin = 5;
int echo_pin   = 18;

    float totalDepth = 1.5; // Total depth of the tank in m
    float tankLength = 2; // Length of the tank in m
    float tankWidth = 2; // Width of the tank in m
    float tankVolume = 0; // Volume of the tank in liters
    float remainingWater = 0; // Remaining liters in the tank
    float tankLiters = 0; // Total liters in the tank
// Replace with your network credentials
const char* ssid = "IMTIAZ";
const char* password = "Imtiaz832921az";

AsyncWebServer server(80);


String page = "";
float distance_cm;
// Function to format a float number with a specific number of decimal places
String formatFloat(float value, int decimalPlaces) {
  String str = String(value, decimalPlaces);
  int dotIndex = str.indexOf('.');
  if (dotIndex == -1) {
    str += '.';
    dotIndex = str.length() - 1;
  }
  while (str.length() < dotIndex + decimalPlaces + 1) {
    str += '0';
  }
  return str;
}

// Function to calculate the volume of the tank in liters
float calculateTankVolume(float totalDepth, float tankLength, float tankWidth) {
    // Calculate the cross-sectional area of the tank
    float crossSectionalArea = tankLength * tankWidth;

    // Calculate the volume of the tank in liters
    // Assuming the tank is a rectangular prism
    float volume = crossSectionalArea * totalDepth;
    return volume;
}

// Function to calculate the remaining liters in the tank based on water depth
float calculateRemainingLiters(float distance_cm, float totalDepth, float tankVolume) {
    // Convert the distance from cm to meters
    if (distance_cm <= 20) {
        distance_cm = 0;
    }
    else if (distance_cm > totalDepth * 100) {
        distance_cm = totalDepth * 100;
    }
    float depth_m = distance_cm / 100.0;
  
   if (depth_m > totalDepth) {
        depth_m = totalDepth;
    }
    // Calculate the remaining volume in the tank in cubic meters
    float remainingVolume_m3 = tankVolume * (1 - (depth_m / totalDepth));

    // Convert the remaining volume from cubic meters to liters
    float remainingLiters = remainingVolume_m3 * 1000;

    return remainingLiters;
}


void setup() {
  Serial.begin(115200);
  pinMode(trigger_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  tankVolume = calculateTankVolume(totalDepth, tankLength, tankWidth);
  tankLiters = tankVolume * 1000;
  
  // Initialize SPIFFS
    // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS");
    return;
  }
    // Route for root / page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html"); // Specify the content type
  });
    // Add a new route to handle the reset request
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Perform the reset here
    ESP.restart();
  });

  // Serve static files (CSS, JS, images, etc.)
  server.serveStatic("/", SPIFFS, "/");

  // Start the server
  server.begin();
    ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  Serial.println("Web server started!");
}

void loop() {
  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);
  long duration = pulseIn(echo_pin, HIGH);
  distance_cm = (duration * 0.034) / 2.0;
  Serial.println(distance_cm);
  int remainingWater = calculateRemainingLiters(distance_cm, totalDepth, tankVolume);
  Serial.println("Tank Volume: " + String(tankVolume) + " m3");
  Serial.println("Remaining Water: " + String(remainingWater) + " liters");
  Serial.println("Tank Liters: " + String(tankLiters) + " liters");
  String jsonStr;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["remainingWater"] = remainingWater;
  serializeJson(jsonDoc, jsonStr);
  ws.textAll(jsonStr); // Send the JSON data to all connected WebSocket clients.
  delay(10000); // Wait for 10 seconds before sending data again.
}
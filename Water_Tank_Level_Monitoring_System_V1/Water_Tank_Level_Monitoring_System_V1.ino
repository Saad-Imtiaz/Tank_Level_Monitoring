#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>

// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Constants for ultrasonic sensor
const int trigPin = 2;
const int echoPin = 15;

// Constants for tank parameters
const char* tankSettingsFile = "/tank_settings.json";
int tankDepth = 100; // Default tank depth in centimeters
int tankVolume = 0;  // Tank volume based on user input

// Function prototypes
void handleRoot();
void handleSettings();
void saveSettings();

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Load tank settings from SPIFFS
  File settingsFile = SPIFFS.open(tankSettingsFile, "r");
  if (settingsFile) {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, settingsFile);
    if (!error) {
      tankDepth = doc["depth"];
      tankVolume = doc["volume"];
      Serial.println("Loaded tank settings from SPIFFS:");
      Serial.print("Depth: ");
      Serial.println(tankDepth);
      Serial.print("Volume: ");
      Serial.println(tankVolume);
    }
    settingsFile.close();
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Print ESP32 IP Address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up GPIO pins for ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

// Route handlers
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  handleRoot();
});
server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request) {
  handleSettings();
});

  // Start server
  server.begin();
}

void loop() {
  // Call the server's handler function
  server.handleClient();
}

// Handler for the web server root page
void handleRoot() {
  // Read the distance from the ultrasonic sensor
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Calculate the water level based on the tank depth and distance
  int waterLevel = tankDepth - distance;

  // Generate JSON response
  String json_response;
  StaticJsonDocument<256> doc;
  doc["depth"] = tankDepth;
  doc["volume"] = tankVolume;
  doc["water_level"] = waterLevel;
  serializeJson(doc, json_response);

  // Send the JSON response to the client
  server.send(200, "application/json", json_response);
}

// Handler for updating tank settings
void handleSettings() {
  if (server.hasArg("depth") && server.hasArg("volume")) {
    tankDepth = server.arg("depth").toInt();
    tankVolume = server.arg("volume").toInt();

    // Save settings to SPIFFS
    saveSettings();

    server.send(200, "text/plain", "Settings updated successfully.");
  } else {
    server.send(400, "text/plain", "Invalid request.");
  }
}

// Function to save tank settings to SPIFFS
void saveSettings() {
  File settingsFile = SPIFFS.open(tankSettingsFile, "w");
  if (!settingsFile) {
    Serial.println("Error opening settings file for writing.");
    return;
  }

  // Create a JSON document to save the settings
  StaticJsonDocument<128> doc;
  doc["depth"] = tankDepth;
  doc["volume"] = tankVolume;

  // Serialize JSON to file
  if (serializeJson(doc, settingsFile) == 0) {
    Serial.println("Failed to write to settings file.");
  }

  settingsFile.close();
}

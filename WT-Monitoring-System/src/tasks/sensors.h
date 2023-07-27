#ifndef SENSORS
#define SENSORS

#include <Arduino.h>
#include "../config.h"
#include <AsyncWebSocket.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h> 


extern AsyncWebServer server;
extern AsyncWebSocket ws; 
// Sensor and Parameters Setup 
extern const int trigger_pin;
extern const int echo_pin;

extern float  totalDepth; // Total depth of the tank in m
extern float  tankLength; // Length of the tank in m
extern float  tankWidth; // Width of the tank in m

extern float tankVolume; // Volume of the tank in liters
extern float remainingWater; // Remaining liters in the tank
extern float tankLiters; // Total liters in the tank
extern float distance_cm;

void setupSensors(){
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

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

float get_sensorData()
{
  // Getting the ultrasonic sensor data
 // Taking the average of 10 readings of distance_cm 
  float avg_distance_cm = 0;
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(TRIGGER_PIN, LOW);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    digitalWrite(TRIGGER_PIN, HIGH);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(TRIGGER_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);

    avg_distance_cm += (duration * 0.034) / 2.0;
    vTaskDelay(300 / portTICK_PERIOD_MS);
   }
  avg_distance_cm = avg_distance_cm / 10.0;
 return avg_distance_cm;
}

void updateWaterLevel()
{
  distance_cm = get_sensorData();
  int remainingWater = calculateRemainingLiters(distance_cm, totalDepth, tankVolume);
  #if DEBUG == true
    Serial.println(distance_cm);
    Serial.println("Tank Volume: " + String(tankVolume) + " m3");
    Serial.println("Remaining Water: " + String(remainingWater) + " liters");
    Serial.println("Tank Liters: " + String(tankLiters) + " liters");
    #endif
  String jsonStr;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["remainingWater"] = remainingWater;
  serializeJson(jsonDoc, jsonStr);
  ws.textAll(jsonStr); // Send the JSON data to all connected WebSocket clients.
}

#endif
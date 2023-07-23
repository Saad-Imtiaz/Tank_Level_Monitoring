#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

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

WebServer server(80);

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
    float depth_m = distance_cm / 100.0;

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
  server.on("/", []() {
    //page = "<head><meta http-equiv=\"refresh\" content=\"3\"></head><center><h1>Ultasonic Water Level Monitor</h1><h3>Current water level:</h3> <h4>" + String(distance_cm) + "</h4></center>";
   // page = "<head><meta http-equiv=\"refresh\" content=\"3\"></head><center><h1>Ultrasonic Water Level Monitor</h1><div style=\"width: 40px; height: 300px; background-color: #f0f0f0; border: 1px solid #999; position: relative;\"><div style=\"width: 100%; background-color: #007bff; position: absolute; bottom: 0; height: " + String((distance_cm / totalDepth) * 100) + "%;\"></div></div><div style=\"display: flex; justify-content: space-between; margin-top: 10px; font-size: 12px; font-weight: bold;\"><span>0 cm</span><span>" + String(totalDepth / 2) + " cm</span><span>" + String(totalDepth) + " cm</span></div><center><h3>Current water level:</h3><h4>" + String(distance_cm) + " cm</h4><h3>Amount of water in the tank:</h3><h4>" + String(((distance_cm / totalDepth) * tankVolume), 2) + " liters</h4></center></center>";
page = "<head><meta http-equiv=\"refresh\" content=\"3\"><link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap\" rel=\"stylesheet\"><style>body{font-family:'Inter', Arial, sans-serif;background-color:#f8f9fa;margin:0;}.container{max-width:600px;margin:0 auto;padding:20px;}.water-tank{width:100px;height:250px;background-color:#f0f0f0;border:1px solid #999;position:relative;margin:20px auto;}.water-level{width:100%;background-color:#007bff;position:absolute;bottom:0;}.water-level-labels{display:flex;justify-content:space-between;margin-top:10px;font-size:12px;font-weight:bold;}</style></head><body><div class=\"container\"><div class=\"text-center\"><h1 class=\"mb-3\", text-align=\"center\">Water Tank Level Monitor</h1></div><div class=\"water-tank\"><div class=\"water-level\" style=\"height: " + String(100 - ((distance_cm / totalDepth) * 100)) + "%;\"></div></div><div class=\"water-level-labels\"><span>" + String(totalDepth / 2) + " cm</span></div><div class=\"text-center mt-3\"><h3>Current Water level:</h3><h4>" + String(distance_cm) + " cm</h4><h3>Water in Tank (Liters):</h3><h4>" + remainingWater + " /" + tankLiters + "/ liters</h4></div></div></body>";

    server.send(200, "text/html", page);
  });
  server.begin();
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
  float remainingWater = calculateRemainingLiters(distance_cm, totalDepth, tankVolume);
  Serial.println("Tank Volume: " + String(tankVolume) + " m3");
  Serial.println("Remaining Water: " + String(remainingWater) + " liters");
  Serial.println("Tank Liters: " + String(tankLiters) + " liters");
  server.handleClient();
  delay(3000);
}
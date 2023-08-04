#ifndef CONFIG
#define CONFIG

#include <Arduino.h>

#define DEBUG true
#define FIREBASE true
#define UPLOAD_LED true
#define RESTART_TIME 3600000 // 1 hr - 3600000 | 2 hr - 7200000
#define BUILTIN_LED 2

#define TRIGGER_PIN 5
#define ECHO_PIN 18

#define TOTAL_DEPTH 0.711; // Total depth of the tank in m
#define TANK_LENGHT 2.463; // Length of the tank in m
#define TANK_WIDTH  2.438;

// Firmware Version :
String FIRMWARE_VERSION = "1-01";
String FIRMWARE_FILE_NAME = "Firmware-" + FIRMWARE_VERSION + ".bin";
String FIRMWARE_FUTURE_FILE_NAME = "Firmware-1-02.bin";

#define FIREBASE_REALTIME_INTERVAL 120000 // 1 min = 60000ms
#define FIREBASE_DATABASE_INTERVAL 60000  // 5 mins = 300000


// Insert Firebase project API Key
#define API_KEY "AIzaSyApbq48CVlSw7ePOPBtnFk8-1zwKMywKAs"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "watertank-os"

#define DATABASE_URL "https://watertank-os-default-rtdb.europe-west1.firebasedatabase.app"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "wateriot@test.com"
#define USER_PASSWORD "123456789"

#define STORAGE_BUCKET_ID "watertank-os.appspot.com"

#endif
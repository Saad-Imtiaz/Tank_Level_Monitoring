#ifndef FIREBASEOTA
#define FIREBASEOTA

#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// Firmware Version :
extern String FIRMWARE_VERSION;
extern String FIRMWARE_FILE_NAME;
extern String FIRMWARE_FUTURE_FILE_NAME;

// Define Firebase Data object
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

extern String ESPID;
extern unsigned int status;
extern unsigned long dataMillis2;
extern bool taskCompleted;
extern bool taskcomplete;

extern short int counter;
extern bool signupOK;
extern short int connected;

extern float tankVolume;
extern float tankLiters;
extern float remainingWater;

void sendFirebaseDatabase() // void *parameter)
{
#if FIREBASE == true
// Firebase Real Time >> Send to Real Time Tracking for each device in FireStore Database
    if (Firebase.ready() && (millis() - dataMillis2 > FIREBASE_REALTIME_INTERVAL || dataMillis2 == 0))
    {
        dataMillis2 = millis();
        taskcomplete = false;

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content2;

        // aa is the collection id, bb is the document id.
        String documentPath2 = "WaterTankIoT/" + String(ESPID) + "/DeviceData";

        if (taskcomplete == false)
        {   content2.set("fields/TankVolume/doubleValue", tankVolume);
            content2.set("fields/TankLiters/doubleValue", tankLiters);
            content2.set("fields/RemainingWater/doubleValue", remainingWater);

            content2.set("fields/CurrentVersion/stringValue", FIRMWARE_FILE_NAME);
            content2.set("fields/FutureVersion/stringValue", FIRMWARE_FUTURE_FILE_NAME);

            Serial.print("Create a document... ");

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath2.c_str(), content2.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
            taskcomplete = true;
        }

        /** if updateMask contains the field name that exists in the remote document and
         * this field name does not exist in the document (content), that field will be delete from remote document
         */
        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath2.c_str(), content2.raw(), "TankVolume,TankLiters,RemainingWater,CurrentVersion,FutureVersion" /* updateMask */))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
#if DEBUG == true
        Serial.println();
        Serial.println("[FIREBASE] Real Time Values Updating");
#endif
    }

#endif
}

// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
    if (info.status == fb_esp_fcs_download_status_init)
    {
        Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
    }
    else if (info.status == fb_esp_fcs_download_status_download)
    {
        Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_fcs_download_status_complete)
    {
        Serial.println("Update firmware completed.");
        Serial.println();
        Serial.println("Restarting...\n\n");
        ESP.restart();
    }
    else if (info.status == fb_esp_fcs_download_status_error)
    {
        Serial.printf("Download firmware failed, %s\n", info.errorMsg.c_str());
    }
}

void firebaseOTA()
{
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        // If you want to get download url to use with your own OTA update process using core update library,
        // see Metadata.ino example

        Serial.println("\nDownload firmware file...\n");

        // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
     
            if (!Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FIRMWARE_FUTURE_FILE_NAME /* path of firmware file stored in the bucket */, fcsDownloadCallback /* callback function */))
        {
            Serial.println(fbdo.errorReason());
            Serial.println(" OTA Status - Unsucessful ");
        }  
      
    }
}


#endif
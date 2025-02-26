#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// WiFi Credentials
#define WIFI_SSID "PrivatNett"
#define WIFI_PASSWORD "Jacobo2511"

// Firebase Credentials
#define FIREBASE_HOST "https://bikube-monitor-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyCUKEazdRZzpRAC9hUmN_-aAzJiYR3C554"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;  // Required for authentication
FirebaseConfig config;  // New required structure for Firebase config

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected!");

    // Firebase Setup
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    float sensorValue = random(20, 30);  // Simulated sensor data

    // Path for temperature data and previous temperature
    String pathTemperature = "/sensor/temperature";
    String pathPreviousTemp = "/sensor/previoustemp";
    
    // Check the current list size for temperature
    if (Firebase.getInt(fbdo, pathTemperature + "/count")) {
        int count = fbdo.intData();
        
        if (count >= 50) {
            // Remove the oldest entry by overwriting it with null (or just don't write to it)
            Firebase.set(fbdo, pathTemperature + "/" + String(count - 50), "");  // Remove the oldest value (index 0)
        }

        // Add the new value to the list of temperatures
        if (Firebase.pushFloat(fbdo, pathTemperature, sensorValue)) {
            Serial.println("New temperature added: " + String(sensorValue));

            // Increment the counter of values
            Firebase.setInt(fbdo, pathTemperature + "/count", count + 1);
        } else {
            Serial.println("Firebase Error: " + fbdo.errorReason());
        }
    } else {
        // Initialize the count if there's an issue fetching it
        if (Firebase.setInt(fbdo, pathTemperature + "/count", 0)) {
            Serial.println("Count initialized");
        } else {
            Serial.println("Firebase Error: " + fbdo.errorReason());
        }
    }

    // Send the sensor value to the "sensor/previoustemp" path
    if (Firebase.setFloat(fbdo, pathPreviousTemp, sensorValue)) {
        Serial.println("Previous temperature set: " + String(sensorValue));
    } else {
        Serial.println("Firebase Error: " + fbdo.errorReason());
    }

   delay(3600000);  // Delay for 1 hour (3,600,000 milliseconds)
}

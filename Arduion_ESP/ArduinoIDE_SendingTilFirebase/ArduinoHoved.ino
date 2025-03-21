#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <OneWire.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

////////////////////////////////////////////
// WiFi Credentials
////////////////////////////////////////////
#define WIFI_SSID "Jacob"
#define WIFI_PASSWORD "12345678"

////////////////////////////////////////////
// Firebase Credentials
////////////////////////////////////////////
#define FIREBASE_HOST "https://bikube-monitor-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyCUKEazdRZzpRAC9hUmN_-aAzJiYR3C554"

////////////////////////////////////////////
// ThingSpeak Credentials
////////////////////////////////////////////
const char* THINGSPEAK_API_KEY = "REHEU1FGRAGQ9FHU";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

////////////////////////////////////////////
// Firebase Objects
////////////////////////////////////////////
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

////////////////////////////////////////////
// OneWire Setup for DS18B20 Temperature Sensor
////////////////////////////////////////////
#define ONE_WIRE_BUS 14
OneWire ds(ONE_WIRE_BUS);

////////////////////////////////////////////
// HX711 Load Cell Setup
////////////////////////////////////////////
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;

////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////
void connectToWiFi();
void setupFirebase();
float readTemperature();
float readWeight();
void sendToFirebase(float temperature, float weight);
void sendToThingSpeak(float temperature, float weight);
void calibrateLoadCell();

////////////////////////////////////////////
// Setup Function
////////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    connectToWiFi();
    setupFirebase();
    LoadCell.begin();
    unsigned long stabilizingtime = 2000;
    LoadCell.start(stabilizingtime, true);
    if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
        Serial.println("HX711 Timeout! Check wiring.");
        while (1);
    }
    LoadCell.setCalFactor(23.18);  // Set the known calibration factor here
    Serial.println("Load cell initialized.");
    while (!LoadCell.update());
    // No need to call calibrateLoadCell() since we use a known offset
}

////////////////////////////////////////////
// Main Loop Function
////////////////////////////////////////////
void loop() {
    float temperature = readTemperature();
    float weight = readWeight();
    
    if (temperature != -127.0) {
        sendToFirebase(temperature, weight);
        Serial.println(temperature);
        sendToThingSpeak(temperature, weight);
        
        Serial.println(weight);
    } else {
        Serial.println("Invalid sensor reading. Skipping data transmission.");
    }
    delay(10000);
}

////////////////////////////////////////////
// Function to Connect to WiFi
////////////////////////////////////////////
void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi!");
}

////////////////////////////////////////////
// Function to Setup Firebase
////////////////////////////////////////////
void setupFirebase() {
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

////////////////////////////////////////////
// Function to Read Temperature
////////////////////////////////////////////
float readTemperature() {
    byte data[9], addr[8];
    float celsius;
    if (!ds.search(addr)) {
        ds.reset_search();
        return -127.0;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);
    delay(1000);
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);
    for (int i = 0; i < 9; i++) {
        data[i] = ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    celsius = (float)raw / 16.0;
    return celsius;
}

////////////////////////////////////////////
// Function to Read Weight
////////////////////////////////////////////
float readWeight() {
    LoadCell.update();
    return LoadCell.getData();
}

////////////////////////////////////////////
// Function to Send Sensor Data to Firebase
////////////////////////////////////////////
void sendToFirebase(float temperature, float weight) {
    String pathTemperature = "/sensor/temperature";
    String pathWeight = "/sensor/weight";
    Firebase.setFloat(fbdo, pathTemperature, temperature);
    Firebase.setFloat(fbdo, pathWeight, weight);
    Serial.println("Data sent to Firebase.");
}

////////////////////////////////////////////
// Function to Send Sensor Data to ThingSpeak
////////////////////////////////////////////
void sendToThingSpeak(float temperature, float weight) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient client; // Create a WiFiClient instance
        String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY + "&field1=" + String(temperature) + "&field2=" + String(weight);
        
        http.begin(client, url); // Use the new API with WiFiClient

        int httpCode = http.GET();
        if (httpCode == 200) {
            Serial.println("Data sent to ThingSpeak.");
        } else {
            Serial.println("Failed to send data to ThingSpeak.");
        }
        http.end();
    } else {
        Serial.println("WiFi disconnected!");
    }
}

////////////////////////////////////////////
// Function to Calibrate Load Cell
////////////////////////////////////////////
void calibrateLoadCell() {
    // Removed manual calibration code since we use a fixed calibration factor
    Serial.println("Load cell calibrated with fixed factor: 23.18");
}

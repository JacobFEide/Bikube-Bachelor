///////////////// Start på kode

#include <ESP8266HTTPClient.h>

#include <ESP8266WiFi.h>

#include <HX711_ADC.h>

#if defined(ESP8266) || defined(ESP32) || defined(AVR)

#include <EEPROM.h>

#include <OneWire.h>

#endif



////////////////////////////////////////////

// WiFi Credentials

////////////////////////////////////////////

#define WIFI_SSID "HyttewifiM"

#define WIFI_PASSWORD "Fjelly714"



////////////////////////////////////////////

// ThingSpeak Credentials

////////////////////////////////////////////

const char* THINGSPEAK_API_KEY = "REHEU1FGRAGQ9FHU";

const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";



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



const int addr_magic = 0;  // EEPROM address for magic number

const int addr_calibration = 4;  // EEPROM address for calibration factor

const int addr_offset = 8;  // EEPROM address for tare offset

const int addr_wake_counter = 12;  // EEPROM address for wake counter

const int max_wake_count = 2;  // Number of wakeups before sending



////////////////////////////////////////////

// Relay Pin

////////////////////////////////////////////

const int relayPin = D6;  // GPIO12



////////////////////////////////////////////

// Function Prototypes

////////////////////////////////////////////

void connectToWiFi();

float readWeight();

float readTemperature();

void sendToThingSpeak(float weight);

void EEPROMWriteFloat(int address, float value);

float EEPROMReadFloat(int address);



////////////////////////////////////////////

// Setup Function

////////////////////////////////////////////

void setup() {

  Serial.begin(115200);



  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, HIGH);  // Start with relay on



  connectToWiFi();



  LoadCell.begin();

  unsigned long stabilizingtime = 2000;

  LoadCell.start(stabilizingtime, false);



  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {

    Serial.println("HX711 Timeout! Check wiring.");

    while (1)
      ;
  }



  EEPROM.begin(512);

  const byte MAGIC_NUMBER = 0x42;

  byte storedFlag = EEPROM.read(addr_magic);



  if (storedFlag != MAGIC_NUMBER) {

    Serial.println("First-time startup detected. Performing tare...");



    while (!LoadCell.update())
      ;

    LoadCell.setCalFactor(1.0);  // Temporary, real calibration later

    LoadCell.tare();  // Tare with current load



    float tareOffset = LoadCell.getTareOffset();

    float calFactor = 20.62;  // <-- Change this to your actual calibration factor



    EEPROM.write(addr_magic, MAGIC_NUMBER);

    EEPROMWriteFloat(addr_offset, tareOffset);

    EEPROMWriteFloat(addr_calibration, calFactor);

    EEPROM.commit();



    Serial.println("Tare and calibration saved to EEPROM.");

  } else {

    float tareOffset = EEPROMReadFloat(addr_offset);

    float calFactor = EEPROMReadFloat(addr_calibration);



    LoadCell.setCalFactor(calFactor);

    LoadCell.setTareOffset(tareOffset);



    Serial.println("Tare and calibration loaded from EEPROM.");
  }



  Serial.println("Load cell initialized.");

  while (!LoadCell.update())
    ;
}



////////////////////////////////////////////

// Main Loop Function

////////////////////////////////////////////

void loop() {

  // Les teller fra EEPROM

  byte wakeCounter = EEPROM.read(addr_wake_counter);



  if (wakeCounter >= max_wake_count) {

    digitalWrite(relayPin, HIGH);

    delay(200);



    float temperature = readTemperature();

    float weight = readWeight();

    Serial.println("Weight: " + String(weight) + " g");



    if (temperature != -127.0) {

      Serial.println("Temp: " + String(temperature) + " °C");

      sendToThingSpeak(temperature, weight);

    } else {

      Serial.println("Ugyldig temperatursensor-avlesning.");
    }



    digitalWrite(relayPin, LOW);  // Slå av releet



    // Nullstill telleren etter sending

    wakeCounter = 0;

    EEPROM.write(addr_wake_counter, wakeCounter);

    EEPROM.commit();

  } else {

    Serial.println("Sender ikke data denne gangen. Teller: " + String(wakeCounter));

    wakeCounter++;

    EEPROM.write(addr_wake_counter, wakeCounter);

    EEPROM.commit();
  }



  WiFi.disconnect();

  delay(1000);

  Serial.println("Sover i 60 minutter...");

  ESP.deepSleep(60000000);  // 60 min i mikrosekunder
}





////////////////////////////////////////////

// Connect to WiFi

////////////////////////////////////////////

void connectToWiFi() {

  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");

    delay(500);
  }

  Serial.println("\nWiFi connected!");
}



////////////////////////////////////////////

// Les temperatur fra DS18B20

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

  ds.write(0x44, 1);  // Start conversion

  delay(1000);

  ds.reset();

  ds.select(addr);

  ds.write(0xBE);  // Read Scratchpad

  for (int i = 0; i < 9; i++) {

    data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];

  celsius = (float)raw / 16.0;

  return celsius;
}





////////////////////////////////////////////

// Read weight from HX711

////////////////////////////////////////////

float readWeight() {
  //Read initial weight some times to stabilice HX711

  int count = 30;
  while (count > 0)  // repeat until count is no longer greater than zero
  {

    if (LoadCell.update()) {

      Serial.println("Stabelizing : " + String(LoadCell.getData()) + " g");  // Return weight data from HX711
      count = count - 1;
    }
  }

  LoadCell.update();

  return LoadCell.getData();  // Return weight data from HX711
}



////////////////////////////////////////////

// Send data til ThingSpeak

////////////////////////////////////////////

void sendToThingSpeak(float temperature, float weight) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    WiFiClient client;



    String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY +

                 "&field1=" + String(temperature) + "&field2=" + String(weight);



    http.begin(client, url);

    int httpCode = http.GET();

    if (httpCode == 200) {

      Serial.println("✅ Data sendt til ThingSpeak.");

    } else {

      Serial.println("❌ Feil ved sending til ThingSpeak.");
    }

    http.end();

  } else {

    Serial.println("❌ Ikke koblet til WiFi.");
  }
}



////////////////////////////////////////////

// Save float value to EEPROM

////////////////////////////////////////////

void EEPROMWriteFloat(int address, float value) {

  byte* data = (byte*)(void*)&value;

  for (int i = 0; i < 4; i++) {

    EEPROM.write(address + i, data[i]);
  }
}



////////////////////////////////////////////

// Read float value from EEPROM

////////////////////////////////////////////

float EEPROMReadFloat(int address) {

  byte data[4];

  for (int i = 0; i < 4; i++) {

    data[i] = EEPROM.read(address + i);
  }

  float value;

  memcpy(&value, data, sizeof(value));

  return value;
}

///////////// slutt på kode
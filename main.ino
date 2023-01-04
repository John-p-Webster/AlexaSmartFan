// Code by John Webster

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif

#include <SinricPro.h>
#include "Fan.h"

#define APP_KEY    "fdba165a-f595-4b50-b55c-f85ba324f30f"
#define APP_SECRET "1e998e21-aa99-4a2a-83f1-a9f030421039-eb267bd5-56e0-4c6c-9511-28a665e10754"
#define DEVICE_ID  "6328f9a1fa69c39e7ccead0d"

#define SSID       "Webster's Wifi"
#define PASS       "2167916155"
#define BAUD_RATE  9600

Fan &fan = SinricPro[DEVICE_ID];

// Variables

// Pin Numbers
const int lightPowerPin = 12;
const int fanPin = 16;
const int colorPin = 17;

// RangeController
std::map<String, int> globalRangeValues;

// ToggleController
std::map<String, bool> globalToggleStates;

/*************
 * Callbacks *
 *************/
// Finds delta Value
int delta(int targetVal, int globalVal, int range){
  if (targetVal < globalVal){
    return targetVal + range + 1 - globalVal;
  }
  else{
    return targetVal - globalVal;
  }
}

// RangeController
bool onRangeValue(const String &deviceId, const String& instance, int &rangeValue) {
  Serial.printf("[Device: %s]: Value for \"%s\" changed to %d\r\n", deviceId.c_str(), instance.c_str(), rangeValue);
  if (instance == "rangeInstance1"){
    for (int i = 0; i != delta(rangeValue, globalRangeValues[instance], 3); i++){
      digitalWrite(fanPin, LOW);
      delay(250);
      digitalWrite(fanPin, HIGH);
      delay(250);
    }
  }
  else{
    for (int i = 0; i != delta(rangeValue, globalRangeValues[instance], 2); i++){
      digitalWrite(colorPin, LOW);
      delay(500);
      digitalWrite(colorPin, HIGH);
      delay(500);
    }
  }
    
  globalRangeValues[instance] = rangeValue;
  return true;
}

bool onAdjustRangeValue(const String &deviceId, const String& instance, int &valueDelta) {
  globalRangeValues[instance] += valueDelta;
  Serial.printf("[Device: %s]: Value for \"%s\" changed about %d to %d\r\n", deviceId.c_str(), instance.c_str(), valueDelta, globalRangeValues[instance]);
  globalRangeValues[instance] = valueDelta;
  return true;
}

// ToggleController
bool onToggleState(const String& deviceId, const String& instance, bool &state) {
  Serial.printf("[Device: %s]: State for \"%s\" set to %s\r\n", deviceId.c_str(), instance.c_str(), state ? "on" : "off");
  if (globalToggleStates[instance] != state){
    digitalWrite(lightPowerPin, LOW);
    delay(500);
    digitalWrite(lightPowerPin, HIGH);
    delay(500);
  }
  globalToggleStates[instance] = state;
  return true;
}

// Events

// ToggleController
void updateToggleState(String instance, bool state) {
  fan.sendToggleStateEvent(instance, state);
}

// RangeController
void updateRangeValue(String instance, int value) {
  fan.sendRangeValueEvent(instance, value);
}

// Setup

void setupSinricPro() {

  // RangeController
  fan.onRangeValue("rangeInstance1", onRangeValue);
  fan.onAdjustRangeValue("rangeInstance1", onAdjustRangeValue);
  fan.onRangeValue("rangeInstance2", onRangeValue);
  fan.onAdjustRangeValue("rangeInstance2", onAdjustRangeValue);


  // ToggleController
  fan.onToggleState("Light", onToggleState);


  SinricPro.onConnected([]{ Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([]{ Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi() {
  WiFi.begin(SSID, PASS);
  Serial.printf("[WiFi]: Connecting to %s", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected\r\n");
}

void setup() {
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, HIGH);
  
  pinMode(lightPowerPin, OUTPUT);
  digitalWrite(lightPowerPin, HIGH);
  
  pinMode(colorPin, OUTPUT);
  digitalWrite(colorPin, HIGH);
  
  Serial.begin(BAUD_RATE);
  setupWiFi();
  setupSinricPro();
}

// Loop

void loop() {
  SinricPro.handle();
}

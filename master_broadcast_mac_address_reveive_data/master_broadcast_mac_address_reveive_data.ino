/*
 * Main Program is Receiver Master
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ArduinoJson.h>
//*** Variable declaration ***
String number_of_devices[20];
typedef struct struct_message {
  String id;
  String mac;
  float temperature;
  float humidity;
  int battery;
} struct_message;
struct_message incomingReadings;

void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  char macaddr[18];
  snprintf(macaddr, sizeof(macaddr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["id"]          = incomingReadings.id;
  root["mac"]         = macaddr;
  root["temperature"] = incomingReadings.temperature;
  root["humidity"]    = incomingReadings.humidity;
  root["battery"]     = incomingReadings.battery;
  root.printTo(Serial);
  Serial.print("\n");

}

//*** End ***
void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP); 
  WiFi.disconnect();
  WiFi.softAP("**********mi88", nullptr, 1);
  WiFi.softAPdisconnect(false);
  Serial.println("THIS NODE MAC ADDRESS IS :");
  Serial.println(WiFi.softAPmacAddress());
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  Serial.printf("MAC: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing protocol");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

}

void loop() {
  // put your main code here, to run repeatedly:

}

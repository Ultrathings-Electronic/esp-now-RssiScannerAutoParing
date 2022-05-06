#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <EEPROM.h>
int addr = 23;
// adding eeprom
//variable delecaration-----
String ssid;
int32_t rssi;
uint8_t encryptionType;
uint8_t* bssid;

int32_t channel;
bool hidden;
int scanResult;
static const int BUTTON_PIN = 0;
static const int LED_PIN = 2;
int ledState = HIGH;
uint8_t peer[18];
Ticker ticker;
bool state = false;
typedef struct message {
  String id;
  float temperature;
  float humidity;
  int battery;
} message;
struct message sendMessage;

void tick() {
  int state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
}

void onSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("SEND : ");
  if (sendStatus == 0) {
    ticker.attach(0.2, tick);
  }
  else {
    ticker.detach();
    digitalWrite(LED_PIN, LOW);
  }
  Serial.println(sendStatus);

}
//-----end------------------
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(LED_PIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  ticker.attach(0.5, tick);
  delay(1000);
  if (esp_now_init() != 0) {
    Serial.println("Error initialization protocol");
    return;
  }

  if (EEPROM.read(addr) == 0) {
    Serial.println("");
    Serial.println("THER IS NO DATA");
  }
  else {
    Serial.print("DATA : ");
    Serial.println(EEPROM.read(addr));
    for (int mac_size = 0; mac_size <= 6; mac_size++) {
      peer[mac_size] = EEPROM.read(addr + mac_size);
      bssid = (uint8_t *)peer;
      //  Serial.println(bssid[mac_size]);
    }
  }
  Serial.print("BSSID LENGTH : ");
  Serial.println(sizeof(peer));
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  Serial.println("Registering a peer device");
  esp_now_add_peer(bssid, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  Serial.println("Registering send callback function");
  esp_now_register_send_cb(onSent);
  ticker.detach();
}

void loop() {
  sendMessage.id = "4";
  sendMessage.temperature = 32;
  sendMessage.humidity = 20;
  sendMessage.battery = 80;

  if (!state) {
    Serial.println(state);
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("LOW");
      ticker.attach(0.8, tick);
      delay(2000);
      scanResult = WiFi.scanNetworks(false, true);
      if (scanResult == 0) {
        Serial.println(F("No networks found"));

      } else if ( scanResult > 0) {
        for ( int i = 0; i < scanResult; i++) {
          ticker.attach(0.1, tick);
          delay(2000);
          WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);
          if (rssi > -20 && channel == 1) {
            Serial.printf(PSTR("[ch %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm"), channel, bssid[0], bssid[1], bssid[2],
                          bssid[3], bssid[4], bssid[5],
                          rssi);
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
            delay(10000);
            Serial.println("SAVE DATA TO EEPROM");
            for (int mac_size = 0 ; mac_size <= 6 ; mac_size++) {
              EEPROM.write(addr + mac_size, bssid[mac_size]);
              Serial.println(mac_size);
              delay(100);
            }
            if (EEPROM.commit()) {
              Serial.println("EEPROM successfully committed");
            } else {
              Serial.println("ERROR Commited");
            }
            delay(1000);
            state = true;
            ESP.restart();
          }
        }

      }
      while (digitalRead(BUTTON_PIN) == LOW);
    }
  }
  if (esp_now_is_peer_exist(bssid) == 0 ) {
    Serial.println("There is no device in a peer list");
  }
  else {
    Serial.println("Send a new message");
    esp_now_send(NULL, (uint8_t *) &sendMessage, sizeof(sendMessage));
  }
  ticker.detach();
  digitalWrite(LED_PIN, LOW);
  delay(2000);
}

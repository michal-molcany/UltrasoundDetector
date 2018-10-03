#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "WiFiManager.h"


#define occupiedPin D7
#define freePin D6

bool failed = false;


ESP8266WiFiMulti WiFiMulti;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  pinMode(occupiedPin, OUTPUT);
  pinMode(freePin, OUTPUT);
  digitalWrite(occupiedPin, HIGH);
  digitalWrite(freePin, HIGH);
  Serial.begin(9600);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

 WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  
  digitalWrite(occupiedPin, LOW);
  digitalWrite(freePin, LOW);
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;
    http.begin("http://10.201.92.39/data"); //HTTP
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        if (payload == "1")
        {
          digitalWrite(occupiedPin, HIGH);
          digitalWrite(freePin, LOW);
        }
        else
        {
          digitalWrite(occupiedPin, LOW);
          digitalWrite(freePin, HIGH);
        }
        failed = false;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      if(failed)
      {
        digitalWrite(occupiedPin, LOW);
        digitalWrite(freePin, LOW);
      }
      failed = true;
    }

    http.end();
  }

  delay(5000);
}


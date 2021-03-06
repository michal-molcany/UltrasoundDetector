#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#include <DNSServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager


const int refreshTime = 3000;

const byte arrayLenght = 6;
const int EEPROMaddress = 0;

ESP8266WebServer server(80);

int ledPin = 14;                // choose the pin for the LED
long duration, distance; // Duration used to calculate distance
bool motion = false;
#define echoPin D7 // Echo Pin
#define trigPin D6 // Trigger Pin
int previous = 0;
byte triggeringDistance = 50;

bool valuesArray[arrayLenght];
byte arrayIndex = 0;

void setArray(bool value)
{
  valuesArray[arrayIndex] = value;
  arrayIndex++;
  if (arrayIndex == arrayLenght)
  {
    arrayIndex = 0;
  }
}
bool getFilterdData()
{
  byte result = 0;
  for (int i = 0; i < arrayLenght; i++)
  {
    result += valuesArray[i];
  }
  if (result > arrayLenght / 2)
    return true;
  else
    return false;
}

void handleRoot() {
  String content = "<html><body>";
  if (motion)
    content += "<H1 style=\"color:rgb(190, 0, 0);\">Game room is occupied.</H1>";
  else
    content += "<H1 style=\"color:rgb(0, 110, 0);\">Game room is free.</H1>";
  content += "Distance: ";
  if (distance == 0)
    content += "Out of range";
  else
    content += distance;
  content += "</body></html>";

  server.send(200, "text/html", content);
}
void handleData()
{
  server.send(200, "text/html", String(getFilterdData()));
}
void handleUpdate()
{
  server.send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup(void) {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);      // declare LED as output
  Serial.begin(9600);
  EEPROM.begin(512);
  byte value = EEPROM.read(EEPROMaddress);
  if (value != 0)
  {
    triggeringDistance = value;
  }

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");


  }

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.on("/update", HTTP_POST, handleUpdate);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  int current = millis() / refreshTime;
  if ( current != previous)
  {
    previous = current;
    checkMovement();
  }

}

void checkMovement() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
  Serial.println(distance);
  if (distance != 0 && distance < triggeringDistance)
  {
    Serial.println("Motion detected!");
    motion = true;
  }
  else
  {
    Serial.println("No motion detected!");
    motion = false;
  }
  ledPin = motion;
  setArray(motion);
  delay(50);
}

// This example uses an Arduino Nano 33 IoT together with
// a WiFiNINA to connect to shiftr.io.

#include <stdio.h>
#include <WiFi.h>
#include <MQTT.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
const char* ssid = "XXX";     //Update with your WiFi SSID
const char* pass = "XXX";     //Update with your WiFi password
const char mqttBroker[63] = "XXX"; 
const char gNumber[5] = "5779";

unsigned long currMillis, prevMillis;
char text_buffer[64];
String oledline[9];
const int oledLib = 1;

WiFiClient wifiClient;
MQTTClient mqttClient;

int myRand;
char mqttClienName[31];

char topic_temp[61]  = "";
char topic_humid[61]  = "";
char msg_temp[64];
char msg_humid[64];
float tempF = 0, humidity = 0;

#define DHTPIN 3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Setup Started");
  randomSeed(1);
  sprintf(topic_temp,  "%s/Gxxxx%s/esp32c3/dht22/temp",  "ece508", gNumber );
  sprintf(topic_humid, "%s/Gxxxx%s/esp32c3/dht22/humid", "ece508", gNumber );

  //Generate Random MQTT ClientID
  myRand = random(0, 9999);
  sprintf(mqttClienName, "client_%04d_Gxxxx%s", myRand, gNumber);
  
  Serial.println("Connecting to Wifi...");
  prevMillis = millis();
  
  WiFi.mode(WIFI_STA);
  int status = WL_IDLE_STATUS;
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {    
    Serial.println("Reattemping WIFI Connect"); delay(250);
  }
  sprintf(text_buffer, "Connected in %d sec", (millis()-prevMillis)/1000);
  Serial.println( String(text_buffer) );

  mqttClient.begin(mqttBroker, wifiClient);

  prevMillis = millis();
}

void loop() {
  mqttClient.loop();

  if (!mqttClient.connected()) {
    Serial.println(String("Connecting to MQTT..."));
    connectMqtt(mqttClienName);
  }

  currMillis = millis();
  // publish a message roughly every 3 second.
  if (currMillis - prevMillis > 3000) {
    Serial.println("Loop: " + String(currMillis-prevMillis));
    prevMillis = currMillis;

    // Read temp/humid sensors
    humidity = dht.readHumidity();
    tempF = dht.readTemperature(true);

    // Store in message
    sprintf(msg_temp, "%.3fF", tempF );
    sprintf(msg_humid, "%.3fRH", humidity);
    Serial.println("Read Temp: " + String(msg_temp) );
    Serial.println("Read Humid:" +  String(msg_humid) );

    mqttClient.publish(topic_temp,  String(msg_temp));
    mqttClient.publish(topic_humid, String(msg_humid));
  }
}

void connectMqtt(char *mqttClienName) 
{
  Serial.println("connectMqtt: Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(1000);
  }
  Serial.println("connectMqtt: WiFi Ok...");
 
  Serial.println("connectMqtt: Checking MQTT...");
  while (!mqttClient.connect(mqttClienName, "public", "public")) {
    Serial.print("."); delay(1000);
  }
  Serial.println("connectMqtt: MQTT Ok...");

}


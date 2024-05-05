/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT and ECE508 Breakout Board
Assignment: HW Week 9 - Nano33IoT, Cloud MQTT, Telegraf 
Description: Publish JSON messages to test.mosquitto.org using MQTT
Issues: None
**************************************************************************/


#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include "myiot33_library.h"
#include <WiFiNINA.h>
//MQTT library for Arduino by Joel Gaehwiler
//Arduino Nano 33 IoT library in https://github.com/256dpi/arduino-mqtt
//Download "arduino-mqtt-master.zip" and install using Add .ZIP library
#include <MQTT.h>
#include <Arduino_JSON.h>


const char gNumber[15] = "Gxxxx5779";  //Update with your G-Number
const char ssid[31] = "XXX";     //Update with your WiFi SSID
const char pass[31] = "XXX";     //Update with your WiFi password

unsigned long currMillis, prevMillis;
char tmpBuffer[64];
String oledline[9];
const int oledLib = 1;

WiFiUDP ntpUDP;
WiFiClient wifiClient;
MQTTClient mqttClient;
const char mqttBroker[63] = "test.mosquitto.org"; 

//NTPClient ntpClient(ntpUDP, "id.pool.ntp.org", 0, 60000);
//NTPClient ntpClient(ntpUDP, "time-a-g.nist.gov", 0, 60000);
//NTPClient ntpClient(ntpUDP, "time.windows.com", 0, 60000);
//NTPClient ntpClient(ntpUDP, "time.google.com", 0, 60000);
NTPClient ntpClient(ntpUDP);

int myRand;
char mqttClienName[31] = "client_59999_G12345678";
int intervalMQTT = 0;
long nmrMqttMessages = 0;
String mqttStringMessage;
char topicPub[61]  = "";
long epochTime = 0;
long epochTimeNTP = 0;
char mac[72];

JSONVar sensorObj;

void setup() {
  Serial.begin(115200); delay(500);
  randomSeed(analogRead(A7));
  sprintf(topicPub, "%s%s%s", "gmu/ece508/", gNumber, "/nano33iot");  //ece508/Gxxxx5678/nano33iot

  //Generate Random MQTT ClientID
  myRand = random(0, 9999);
  sprintf(mqttClienName, "client_%04d_%s", myRand, gNumber);
  
  iot33StartOLED(oledLib);
  oledline[1] = String(gNumber) + " Shiftr.io";
  for (int jj=2; jj<=8; jj++){ oledline[jj]=""; }
  
  // attempting to connect to Wifi network:  
  oledline[2] = "Connecting to Wifi..."; displayTextOLED(oledline, oledLib);
  prevMillis = millis();
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {    
    status = WiFi.begin(ssid, pass); delay(250);
  }
  sprintf(tmpBuffer, "Connected in %d sec", (millis()-prevMillis)/1000);
  oledline[2] = tmpBuffer; displayTextOLED(oledline, oledLib); delay(500);

  ntpClient.begin();

  mqttClient.begin(mqttBroker, wifiClient);
  mqttClient.onMessage(messageReceived);

  oledline[2] = "Synchronizing NTP..."; displayTextOLED(oledline, oledLib); delay(500);
  Serial.print("Time Synchronization NTP");
  epochTimeNTP = ntpClient.getEpochTime();
  while (epochTimeNTP > 1680300000) {    
    epochTimeNTP = ntpClient.getEpochTime();    
    Serial.print(".");
    delay(2000);
  }  
  Serial.println(ntpClient.getEpochTime());
  oledline[2] = "NTP Synchronized"; displayTextOLED(oledline, oledLib); delay(500);
  
  getMacWifiShieldMacRouterSS(mac);

  /*
  Serial.print("Time Synchronization...");
  epochTime = WiFi.getTime();
  while (epochTime == 0) {    
    epochTime = WiFi.getTime();    
    Serial.print(".");
    delay(2000);
  }
  Serial.println("\n");
  */
  
  prevMillis = millis();
}

void loop() {
  
  mqttClient.loop();

  if (!mqttClient.connected()) {
    for (int jj=2; jj<=8; jj++){ oledline[jj]=""; }
    oledline[2] = String("Connecting to MQTT..."); displayTextOLED(oledline, oledLib);
    connectMqtt(mqttClienName);
  }

  currMillis = millis();
  // publish a message roughly every second.
  if (currMillis - prevMillis > 1000) {
    ntpClient.update();
    prevMillis = currMillis;

    oledline[2] = String(mqttBroker, 21);
    oledline[3] = String(mqttClienName, 21);
    
    intervalMQTT++;
    if (intervalMQTT >= 5) {   
      intervalMQTT = 0;
      nmrMqttMessages++;
      oledline[4] = String("Send message: ") + String(nmrMqttMessages);
      getMacWifiShieldMacRouterSS(tmpBuffer);
      
      //mqttStringMessage = "airSensors,sensor_id=TLM0100 temperature=71.15875225821217,humidity=35.12865369058315,co=0.5169902572341222 1680040153000000000";
      //epochTime = WiFi.getTime();
      epochTimeNTP = ntpClient.getEpochTime();
      
      sensorObj["measurement"] = "ece508meas";
      sensorObj["device"] = "nano33iot";
      sensorObj["gnumber"] = String(gNumber);
      sensorObj["nmrMqttMessages"] = nmrMqttMessages;
      sensorObj["mac"] = String(mac);
      sensorObj["rssi"] = WiFi.RSSI();
      sensorObj["epoch"] = epochTimeNTP;
      
            
      //mqttStringMessage = String("airSensor,device=nano33iot") + " " +
      //                           "nmrMqttMessages=" + String(nmrMqttMessages) + "," +
      //                           "rssi=" + WiFi.RSSI()  + " " +
      //                    String(epochTimeNTP) + "000000000";

      mqttStringMessage = JSON.stringify(sensorObj);
      
      mqttClient.publish(topicPub, mqttStringMessage);
      Serial.println("Topic:" + String(topicPub) + " Message:" + String(mqttStringMessage));
    }

    epochTimeNTP = ntpClient.getEpochTime();
    convCurrentTimeET_DST(epochTimeNTP, tmpBuffer, 1);
    oledline[7] = String(tmpBuffer);          
    
    convDDHHMMSS(currMillis/1000, tmpBuffer);
    oledline[8] = "Uptime: " + String(tmpBuffer);
    displayTextOLED(oledline, oledLib);
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
  //while (!mqttClient.connect(mqttClienName, "public", "public")) {
  while (!mqttClient.connect(mqttClienName)) {
    Serial.print("."); delay(1000);
  }
  Serial.println("connectMqtt: MQTT Ok...");
 
  //sprintf(topicSubHeartbeat, "%s%s", gNumber, "/heartbeat");
  //mqttClient.subscribe(topicSubHeartbeat);
  //Serial.println("Subscribe: " + String(topicSub));
  //mqttClient.subscribe("ece508/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

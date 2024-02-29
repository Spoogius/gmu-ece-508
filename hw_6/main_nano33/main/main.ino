/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT Dev Kit and ECE508 Breakout Board
Assignment: HW Week 6 - WiFiNINA and WiFi Programming
Description: Connect to a WiFi network and display varous properties of the 
connection on the oLED.
Issues: None
**************************************************************************/

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <WiFiNINA.h>
#include <Arduino_JSON.h>


const char gNumber[11] = "G5779";           //Replace with your last digits of G-number
const char* wifi_ssid = "XXX";        //Replace with your WiFi SSID
const char* wifi_pass = "XXX";        //Replace with your WiFi password

int statusWiFi = WL_IDLE_STATUS;

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire myOled;

long currMillis, prevMillis;
char tmpBuffer[64];
String oledline[9];
char boardWifiMAC[31];
char routerMAC[31];
JSONVar sensorObj;
String stringJson;

void setup() {
  //Initialize serial:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  Wire.setClock(400000L); //10000 (low speed mode), 100000 (standard mode), 400000 (fast mode), 1000000 (fast mode plus), 3400000 (high speed mode)
  myOled.begin(&Adafruit128x64, I2C_ADDRESS);
  myOled.setFont(Adafruit5x7);

  oledline[1] = String(gNumber) + "  WiFiNINA"; //String(boardWifiMAC);
  int jj; for (jj=2; jj<=8; jj++){ oledline[jj]=""; }
  displayTextOLED();

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( statusWiFi != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: " + String(wifi_ssid));   
    statusWiFi = WiFi.begin(wifi_ssid, wifi_pass);
  }

  getMacWifiShield(boardWifiMAC);
  getMacRouter(routerMAC);

}

void loop() {

  currMillis = millis();
  if (currMillis - prevMillis > 1000) {
    prevMillis = currMillis;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    getWiFiRSSI(tmpBuffer);
    oledline[2] = String(tmpBuffer);

    oledline[4] =  "SSID: " + String(wifi_ssid);

    oledline[6] = "rM: " + String(routerMAC);
    oledline[7] = "wM: " + String(boardWifiMAC);
    
    convDDHHMMSS(millis()/1000, tmpBuffer);
    oledline[8] = "Uptime: "+ String(tmpBuffer);
    displayTextOLED();    
  }

}

void displayTextOLED() {
  int jj;
   
  myOled.clear();
  myOled.set1X();
  Serial.println("=====================");
  for (jj = 1; jj <= 8; jj++) {  
     myOled.println(oledline[jj]);
     Serial.println(oledline[jj]);
  }
  Serial.println("=====================");
}

void convDDHHMMSS(unsigned long currSeconds, char *uptimeDDHHMMSS) 
{
  int dd, hh, mm, ss;

  ss = currSeconds; //258320.0 2 23:45:20
  dd = (ss/86400);
  hh = (ss-(86400*dd))/3600; 
  mm = (ss-(86400*dd)-(3600*hh))/60;
  ss = (ss-(86400*dd)-(3600*hh)-(60*mm));

  sprintf(uptimeDDHHMMSS, "%d %02d:%02d:%02d", dd, hh ,mm, ss);
};

void getMacWifiShield(char *macWifiShield) 
{
    byte mac[6];

    WiFi.macAddress(mac);
    sprintf(macWifiShield, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
};

void getMacRouter(char *macRouter) {
  byte mac[6];
  WiFi.BSSID(mac);
  sprintf(macRouter, "%02X:%02X:%02X:%02X:%02X:%02X\0", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
};

void getWiFiRSSI(char *wifiRSSI) 
{
    sprintf(wifiRSSI, "%lddBm %d.%d.%d.%d", WiFi.RSSI(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
};

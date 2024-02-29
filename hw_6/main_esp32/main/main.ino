/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino ESP32C3 Dev Kit and ECE508 Breakout Board
Assignment: HW Week 6 - WiFiNINA and WiFi Programming
Description: Connect to a WiFi network and display varous properties of the 
connection on the oLED.
Issues: None
**************************************************************************/

#include <stdio.h>
#include <WiFi.h>
#include <Arduino_JSON.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_ADDRESS 0x3C
Adafruit_SSD1306 myOled(128, 64, &Wire, -1);

const char gNumber[11] = "G5779\0";           //Replace with your last digits of G-number
const char* wifi_ssid = "XXX";        //Replace with your WiFi SSID
const char* wifi_pass = "XXX";        //Replace with your WiFi password


float tsens_out;
unsigned long currMillis, prevMillis;
char lcdBuffer[64];
String oledline[9];
const int oledLib = 1;
int ledStatus = HIGH;
int ledCnt = 0;

void setup() {


  Wire.begin(4, 5); //SDA, SCL
  if(!myOled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) { 
    Serial.println("SSD1306 allocation failed");
  };
    
  oledline[1] = String(gNumber) + String(" WiFiNINA");
  int jj; for (jj=2; jj<=8; jj++){ oledline[jj]=""; }
  
  WiFi.begin(wifi_ssid, wifi_pass);
  oledline[2] = "Connecting to Wifi..."; displayTextOLED(oledline);
  prevMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {    
    Serial.print("Attempting to connect to WiFi: ");   
    delay(250);
  }
  currMillis = millis();
  sprintf(lcdBuffer, "Connected in %d sec", (currMillis-prevMillis)/1000);
  oledline[2] = lcdBuffer; displayTextOLED(oledline);

  prevMillis = millis();

}

void loop() {

  currMillis = millis();

  if (currMillis - prevMillis >= 1000) //Display every second
    { 
      prevMillis = currMillis;
      
      getWiFiRSSI(lcdBuffer);
      oledline[2] = String(lcdBuffer);

      //oledline[3] = "Temp: " + String(tsens_out) + " C";
      oledline[4] = "SSID: " + String(WiFi.SSID());

      //WiFi.BSSID();
      getMacRouterESP32(lcdBuffer);;
      oledline[6] = "rM: " + String(lcdBuffer);
      getMacWifiShield(lcdBuffer);
      oledline[7] = "wM: " + String(lcdBuffer);
      
      convDDHHMMSS(currMillis/1000, lcdBuffer);
      oledline[8] = "Uptime: " + String(lcdBuffer);
      
      displayTextOLED(oledline);

    }
}

void displayTextOLED(String oledline[9])
{
  int jj;
  
  myOled.clearDisplay();
  myOled.setTextSize(1);
  myOled.setTextColor(SSD1306_WHITE);
  myOled.setCursor(0, 0);

  for (jj=1; jj<=8; jj++) { 
    myOled.println(oledline[jj]);
    }
  
  myOled.display(); 
}

void convDDHHMMSS(unsigned long currSeconds, char *uptimeDDHHMMSS) 
{
  int dd, hh, mm, ss;

  ss = currSeconds; //258320.0 2 23:45:20
  dd = (ss/86400);
  hh = (ss-(86400*dd))/3600; 
  mm = (ss-(86400*dd)-(3600*hh))/60;
  ss = (ss-(86400*dd)-(3600*hh)-(60*mm));

  sprintf(uptimeDDHHMMSS, "%02d %02d:%02d:%02d", dd, hh ,mm, ss);
};

void getWiFiRSSI(char *wifiRSSI) 
{
    sprintf(wifiRSSI, "%lddBm %d.%d.%d.%d", WiFi.RSSI(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
};

void getMacWifiShield(char *macWifiShield) 
{
    byte mac[6];

    WiFi.macAddress(mac);
    sprintf(macWifiShield, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
};

void getMacRouterESP32(char *macRouter)  
{
    sprintf(macRouter, "%02X:%02X:%02X:%02X:%02X:%02X", WiFi.BSSID()[0], WiFi.BSSID()[1], WiFi.BSSID()[2], WiFi.BSSID()[3], WiFi.BSSID()[4], WiFi.BSSID()[5]);
};

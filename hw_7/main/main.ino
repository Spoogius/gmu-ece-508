/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT and ECE508 Breakout Board
Assignment: HW Week 7 - WiFiNINA Client Webserver access
Description: Display times generated from polling a GMU hosted time server
Issues: None
**************************************************************************/

#include <time.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <Arduino_JSON.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

long currMillis, prevMillis;
char tmpBuffer[64];
String oledline[8];

JSONVar web_return;

char wifi_ssid[] = "XXXXX";     // your network SSID (name)
char wifi_pass[] = "XXXXX";    // your network password (use for WPA, or use as key for WEP)

char serverAddress[] = "gmu-ece508.azurewebsites.net";  // server domain address
char serverRequest[] = "/api/gmuepoch?code=V0drkytIHnClqQitBE_oJYJNhugUfzojkRbmq2tL8ZDUAzFu-TCk8Q==";
int serverPort = 443;
WiFiSSLClient client;
int statusWiFi = WL_IDLE_STATUS;

void displayTextOLED_Ascii(String oledline[8]) {
  oled.clear();
  oled.set1X();
  for (int jj = 0; jj < 8; jj++) {  
     oled.println(oledline[jj]);
  }
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

void convCurrentTimeET(unsigned long currSeconds, char *currentTimeET) 
{
    time_t rawtime = currSeconds - 18000 + 3600;
    struct tm  ts;
    char buf[70];

    ts = *localtime(&rawtime);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
    sprintf(currentTimeET, buf);

};

void getWiFiRSSI(char *wifiRSSI) 
{
    sprintf(wifiRSSI, "%lddBm %d.%d.%d.%d", WiFi.RSSI(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
};

JSONVar poll_server( char* address, char* request, int port ){
  String payload;
  String json_payload;

  client.println("GET /api/gmuepoch?code=V0drkytIHnClqQitBE_oJYJNhugUfzojkRbmq2tL8ZDUAzFu-TCk8Q== HTTP/1.1");
  client.println("Host: gmu-ece508.azurewebsites.net");
  client.println();

  while( !client.available() ){ }; // Wait for response
  //client.println("Connection: close");
  
  while (client.available()) {
    payload = client.readString();
    json_payload = payload.substring(payload.lastIndexOf('{'), payload.lastIndexOf('}') + 1);
    //Serial.println( json_payload );
  }
  return JSON.parse(json_payload);
};

void setup() {
  //Initialize serial:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  Wire.setClock(400000L); //10000 (low speed mode), 100000 (standard mode), 400000 (fast mode), 1000000 (fast mode plus), 3400000 (high speed mode)
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);

  oledline[0] = "G5779";
  displayTextOLED_Ascii(oledline);

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

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connectSSL("gmu-ece508.azurewebsites.net", serverPort)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /api/gmuepoch?code=V0drkytIHnClqQitBE_oJYJNhugUfzojkRbmq2tL8ZDUAzFu-TCk8Q== HTTP/1.1");
    client.println("Host: gmu-ece508.azurewebsites.net");
    //client.println("Connection: close");
    client.println();
  }
  currMillis = millis();
  prevMillis = currMillis;
}

void loop(){
  currMillis = millis();
  if( currMillis - prevMillis > 1000 ){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    //Serial.println("Connection Status: " + String(client.connected()) + " Avail: " + String(client.available()));

    web_return = poll_server(serverAddress, serverRequest, serverPort );
    Serial.println(JSON.stringify(web_return));

    char wifiRSSI[70];
    char uptime_string[70];
    char wifitime_string[70];
    getWiFiRSSI(wifiRSSI);
    
    Serial.println("Curr Millis: " + String(currMillis));
    convDDHHMMSS( currMillis/1000, uptime_string);
    Serial.println("Uptime String: " + String(uptime_string));

    long epoch_time = web_return["epoch_seconds"];
    convCurrentTimeET(epoch_time,wifitime_string);

    oledline[0] = "G5779";
    oledline[1] = String(wifiRSSI);
    oledline[2] = "";
    oledline[3] = "";
    oledline[4] = "";
    oledline[5] = String("Server: ") + String(epoch_time);
    oledline[6] = String("Uptime: ") + String(uptime_string);
    oledline[7] = String(wifitime_string);
    displayTextOLED_Ascii(oledline);
    prevMillis = currMillis;
  }
}



/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino ESP32C3 Dev Kit and ECE508 Breakout Board
Assignment: HW Week 5 - Programming OLED Display, Serial command and DHT22
Description: Display temperature and humidity data on the oLED and accept serial commands
to toggle between C & F.
Issues: None
**************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SimpleDHT.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 myOled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String led_text_lines[8]; 
enum temperature_mode{
  OFF = 0,
  C = 1,
  F = 2
};
temperature_mode current_temp_mode = F;

// Init dht22 object to correct pin
int pinDHT22 = 3;
SimpleDHT22 dht22(pinDHT22);
float curr_temperature = 0;
float curr_humidity = 0;
int errDHT22 = SimpleDHTErrSuccess;

// Init MAC read
char mac_address[31];

// Timestamp variables
long prev_millis, curr_millis = 0;
long days, hours, minutes, seconds = 0;

// Serial control variables
int cmd_major = 0;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Make sure to use the correct I2C address. Address 0x3C for 128x64
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  Wire.begin(4, 5); //SDA, SCK
  if(!myOled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  getMacWifiShield( mac_address );
  digitalWrite(LED_BUILTIN, 0);

  update_display( 0, current_temp_mode );
}



void loop() {
  curr_millis = millis();

   while( Serial.available() ){
    cmd_major = Serial.parseInt();
    if( cmd_major != 15 ){ break; }
    current_temp_mode = (temperature_mode)Serial.parseInt();
    // Input validation - Default to OFF if out of range
    if( current_temp_mode != F && current_temp_mode != C ){
      current_temp_mode = OFF;
    }
    char ack_msg[32];
    sprintf(ack_msg, "Ack: %d %d\0", cmd_major, current_temp_mode );
    Serial.println( String(ack_msg) );
    break;
  }

  if( curr_millis - prev_millis >= 1000 ){
    Serial.println("Current Time: " + String( curr_millis ) );
    update_display( curr_millis, current_temp_mode );
    prev_millis = curr_millis;
  }
}

void update_display( long timestamp_ms, temperature_mode mode){


  char line_0[32];
  sprintf( line_0, "G:5779 %s\0", mac_address );

  millis_to_timestamp( timestamp_ms, &seconds, &minutes, &hours, &days );

  char timestamp_string[12];
  sprintf(timestamp_string, "%2d:%2d:%2d:%2d\0", days, hours, minutes, seconds );
  // Zero pad printed digits less than 10.
  timestamp_string[0] = ( days    < 10 ? '0' : timestamp_string[0] );
  timestamp_string[3] = ( hours   < 10 ? '0' : timestamp_string[3] );
  timestamp_string[6] = ( minutes < 10 ? '0' : timestamp_string[6] );
  timestamp_string[9] = ( seconds < 10 ? '0' : timestamp_string[9] );

  led_text_lines[0] = String(line_0);
  led_text_lines[1] = "ECE508 HW05";
  led_text_lines[2] = "";
  led_text_lines[3] = ""; // Temperature in F:
  led_text_lines[4] = ""; // Humidity in RH%:
  led_text_lines[5] = "";
  led_text_lines[6] = "Use Pin D5";
  led_text_lines[7] = "Uptime: " + String( timestamp_string );

  // Read temperature over dht22
  errDHT22 = dht22.read2(&curr_temperature, &curr_humidity, NULL);

  char temp_line[32];
  char humid_line[32];
  switch( mode ){
    case C:
      sprintf( temp_line,  "Temp in C: %3.1fC\0", curr_temperature );
      sprintf( humid_line, "Humidity in RH%: %3.1f\0", curr_humidity );
      break;
    case F:
      curr_temperature = ( curr_temperature*9/5 )+32;
      sprintf( temp_line,  "Temp in F: %3.1fF\0", curr_temperature );
      sprintf( humid_line, "Humidity in RH%: %3.1f\0", curr_humidity );
      break;
    default:
      sprintf( temp_line,  "\0" );
      sprintf( humid_line, "\0" );
      break;
  };
  // Assing to output array
  led_text_lines[3] = String( temp_line ); 
  led_text_lines[4] = String( humid_line );


  // Write to display
  myOled.clearDisplay();
  myOled.setTextSize(1);
  myOled.setTextColor(SSD1306_WHITE);
  myOled.setCursor(0, 0);
  for ( uint8_t jj=0; jj<8; jj++) { 
    myOled.println(led_text_lines[jj]);
  }
  myOled.display();  
};

void getMacWifiShield(char *macWifiShield) 
{
    byte mac[6];

    WiFi.macAddress(mac);
    sprintf(macWifiShield, "%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
};

const long MILLIS_TO_DAYS    = (1000*60*60*24);
const long MILLIS_TO_HOURS   = (1000*60*60);
const long MILLIS_TO_MINUTES = (1000*60);
const long MILLIS_TO_SECONDS = (1000);

void millis_to_timestamp( long millis_since_boot, long* seconds, long* minutes, long* hours, long* days ){

  long millis_remaining = millis_since_boot;

  *days = millis_remaining/MILLIS_TO_DAYS;
  millis_remaining = millis_since_boot - ((*days)*MILLIS_TO_DAYS);

  *hours = millis_since_boot/MILLIS_TO_HOURS;
  millis_remaining = millis_since_boot - ((*hours)*MILLIS_TO_HOURS);

  *minutes = millis_remaining/MILLIS_TO_MINUTES;
  millis_remaining = millis_remaining - ((*minutes)*MILLIS_TO_MINUTES);

  *seconds = millis_remaining/MILLIS_TO_SECONDS;
  millis_remaining = millis_remaining - ((*seconds)*MILLIS_TO_SECONDS);

  return;
};
/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino ESP32-C3 Dev Kit and ECE508 Breakout Board
Assignment: HW Week 4 -  Gartner Paper Review, Arduino and ESP32 Programming
Description: Blink the onboard LED at 1Hz and update the oLED using
the assignment specified format at a 1Hz refresh rate.
Issues: None
**************************************************************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 myOled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long currMillis, prevMillis;
unsigned int ledcnt = 0;
String oledline[9];
int cnt = 0;

uint8_t RGB_LED = RGB_BUILTIN;

void setup() {
  pinMode(RGB_LED, OUTPUT);
  Serial.begin(115200); delay(500);

  Wire.begin(4, 5); //SDA, SCK
  if(!myOled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed...");
    for(;;); // Don't proceed, loop forever
  }

  oledline[1] = "ECE508 02/19/2024";
  oledline[2] = "Gxxxx5779";
  int jj; for (jj=3; jj<=8; jj++){ oledline[jj]=""; }
}

const long MILLIS_TO_HOURS = (1000*60*60);
const long MILLIS_TO_MINUTES = (1000*60);
const long MILLIS_TO_SECONDS = (1000);

void loop() {
 
  currMillis = millis();
  if (currMillis - prevMillis > 500) {
    prevMillis = currMillis;
    //digitalWrite(RGB_LED, (ledcnt++)%2);
    if((ledcnt++)%2){
      neopixelWrite(RGB_LED, 128,0,0);
    } else {

      long millis_since_boot = currMillis;
      // Time Conversion code
      long hours, minutes, seconds, millis_remaining = 0;

      hours = millis_since_boot/MILLIS_TO_HOURS;
      millis_remaining = millis_since_boot - (hours*MILLIS_TO_HOURS);

      minutes = millis_remaining/MILLIS_TO_MINUTES;
      millis_remaining = millis_remaining - (minutes*MILLIS_TO_MINUTES);

      seconds = millis_remaining/MILLIS_TO_SECONDS;
      millis_remaining = millis_remaining - (seconds*MILLIS_TO_SECONDS);
      char timestamp_string[9] = "00:00:00";
      sprintf(timestamp_string, "%2d:%2d:%2d\0", hours, minutes, seconds );
      // Zero pad printed digits less than 10.
      timestamp_string[0] = ( hours   < 10 ? '0' : timestamp_string[0] );
      timestamp_string[3] = ( minutes < 10 ? '0' : timestamp_string[3] );
      timestamp_string[6] = ( seconds < 10 ? '0' : timestamp_string[6] );

      oledline[3] = String(currMillis);
      oledline[4] = "";
      oledline[5] = "";
      oledline[6] = "";
      oledline[7] = ""; 
      oledline[8] = String(timestamp_string);
      displayTextOLED(oledline);
      digitalWrite(RGB_LED, 0);
    }

     
    
    
  }
}

void displayTextOLED(String oledline[]) {
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

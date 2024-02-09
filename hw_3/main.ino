/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT and ECE508 Breakout Board
Assignment: HW Week 3 - Paper Review and Arduino Programing (OLED) 
Description: Blink the onboard LED at 0.5 Hz and update the oLED using
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

String led_text_lines[8]; 


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Make sure to use the correct I2C address. Address 0x3C for 128x64
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!myOled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
  digitalWrite(LED_BUILTIN, 0);
  update_display( 0 );
}

long prev_millis, curr_millis = 0;
long count_500ms = 0;
uint8_t LED_STATUS = 0;

void loop() {
  curr_millis = millis();
  if( curr_millis - prev_millis >= 500 ){
    count_500ms++;
    LED_STATUS = !LED_STATUS;

    digitalWrite(LED_BUILTIN, LED_STATUS);
    Serial.println("Setting LED: " + String(LED_STATUS) );
    if( count_500ms % 2 == 0 ){
        update_display( curr_millis);
        Serial.println("Updating oLED: " + String(curr_millis) + "ms" );
    }
    prev_millis = curr_millis;
  }
}


const long MILLIS_TO_HOURS = (1000*60*60);
const long MILLIS_TO_MINUTES = (1000*60);
const long MILLIS_TO_SECONDS = (1000);

void update_display( long millis_since_boot ){

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

  led_text_lines[0] = "ECE508 02/09/2024"; 
  led_text_lines[1] = "G:xxxx5779";
  led_text_lines[2] = "Up time: " + String(millis_since_boot) + "ms";
  led_text_lines[3] = String( timestamp_string );
  led_text_lines[4] = "Test 5";
  led_text_lines[5] = "Test 6"; 
  led_text_lines[6] = "Test 7";
  led_text_lines[7] = led_text_lines[3];

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

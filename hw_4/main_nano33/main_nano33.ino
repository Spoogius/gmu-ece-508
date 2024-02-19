/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT and ECE508 Breakout Board
Assignment: HW Week 4 -  Gartner Paper Review, Arduino and ESP32 Programming
Description: Blink the onboard LED at 0.5 Hz and update the oLED using
the assignment specified formatted Gyro and Accelerometer values at a 1Hz refresh rate.
Issues: None
**************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino_LSM6DS3.h>

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

  // Init IMU
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU!");
      while (1);
    }
    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");

  digitalWrite(LED_BUILTIN, 0);
  update_display( );

}

long prev_millis, curr_millis = 0;
uint8_t LED_STATUS = 0;

void loop() {
  curr_millis = millis();
  if( curr_millis - prev_millis >= 1000 ){

    LED_STATUS = !LED_STATUS;

    // Update Display
    digitalWrite(LED_BUILTIN, LED_STATUS);
    Serial.println("Setting LED: " + String(LED_STATUS) );
    update_display();
    Serial.println("Updating oLED: " + String(curr_millis) + "ms" );

    prev_millis = curr_millis;
  }
}
float aX, aY, aZ;
float gX, gY, gZ;

void update_display( ){

  // Read hardware values
  if (IMU.accelerationAvailable()) {      
    IMU.readAcceleration(aX, aY, aZ);
  } else {
    Serial.print("Error reading acc values");
  }
  if (IMU.gyroscopeAvailable()) {      
    IMU.readGyroscope(gX, gY, gZ);
  } else {
    Serial.print("Error reading gyro values");
  }

  led_text_lines[0] = "ECE508 02/19/2024"; 
  led_text_lines[1] = "G:xxxx5779";
  // For some reason g & a seem to be reading each others values
  led_text_lines[2] = "IMU aX: " + String(gX);
  led_text_lines[3] = "IMU aY: " + String(gY);
  led_text_lines[4] = "IMU aZ: " + String(gZ);
  led_text_lines[5] = "IMU gX: " + String(aX);
  led_text_lines[6] = "IMU gY: " + String(aY);
  led_text_lines[7] = "IMU gZ: " + String(aZ);


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

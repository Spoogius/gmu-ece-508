/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT and eceNano33IoT-ESP32C3_v02 PCB
Assignment: HW Week 2 - Arduino Programing (Dynamic LED blinking and Serial interface) 
Description: Using the serial input to write controlable LED blink frequencies to the
either the Nano 33 IoT of ESP32-C3 
Issues: None
**************************************************************************/

unsigned long prev_mil_sec, curr_mil_sec;
int led_value;
float cmd;

// Buffers for sprintf format prints
char ack_buffer[256];
char led_change_buffer[256];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT );
  Serial.begin(9600); delay(100);
  
  cmd = 0.5;
  led_value = 1;
  digitalWrite(LED_BUILTIN, led_value );
  prev_mil_sec = millis();

  return;
}

void loop() {

  parse_serial_command();
  
  curr_mil_sec = millis();
  if( (curr_mil_sec-prev_mil_sec) > ( (1000/2)/cmd)){
    led_value = !led_value;
    digitalWrite( LED_BUILTIN, led_value );
    sprintf( led_change_buffer, "LED set to: %d after %lums", led_value, curr_mil_sec - prev_mil_sec);
    Serial.println( led_change_buffer );
    prev_mil_sec = millis();
  }

}

void parse_serial_command( void ){

  while( Serial.available() ){
    cmd = Serial.parseFloat();
    if( Serial.read() == '\n' ){
      sprintf( ack_buffer, "Received Command for LED_BUILTIN set to: %.2fHz", cmd );
      Serial.println( ack_buffer );
      break;
    }
  }

  return;
}
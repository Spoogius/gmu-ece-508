
#include "DHT.h" //Adafruit DHT sensor library
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoMqttClient.h>
#include <Arduino_JSON.h>

#define SWITCHPIN 8
#define CTRLPIN 9
#define DHTPIN 3

const char gNumber[15] = "Gxxxx5779";  //Update with the last digits of your G-number
const char ssid[31] = "XXXX";     //Update with your WiFi SSID
const char pass[31] = "XXXX";     //Update with your WiFi password
WiFiUDP ntpUDP;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
NTPClient ntpClient(ntpUDP);

const char mqttBroker[63] = "test.mosquitto.org";
const int mqttPort = 1883;


char mqttClienName[31] = "client_32_Gxxxx5779";

char topicPub_dht[64]  = "";
char topicSub_ctrl[64]  = "";

JSONVar msg_dht;
char msg_recv_arr[1024];
JSONVar msg_ctrl;

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

bool switch_state;
bool manual_override = false;
bool curr_state = 0;

float curr_time, prev_time = 0;

void setup() {
  pinMode(RGB_BUILTIN, OUTPUT);
  pinMode(SWITCHPIN, INPUT);
  pinMode(CTRLPIN, OUTPUT);

  Serial.begin(115200);
  switch_state = digitalRead(SWITCHPIN);

  prev_time = millis();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {        
    Serial.println("Attempting to connect to WiFi...");
    delay(250);
  }
  Serial.println("Connected to Wifi after: " + String(millis()-prev_time) + "ms");

  ntpClient.begin();
  msg_dht["timestamp"] = ntpClient.getEpochTime(); 
  msg_dht["source"] = "ESP32";
  msg_dht["temp_f"] = 0;
  msg_dht["humidity"] = 0;

  sprintf(topicPub_dht, "%s%s%s", "gmu/ece508/", gNumber, "/dht_data");
  sprintf(topicSub_ctrl, "%s%s%s", "gmu/ece508/", gNumber, "/ctrl_state");

  dht.begin();
}

void loop() {
  curr_time = millis();
  
  mqttClient.poll();
  if (!mqttClient.connected()) {
    Serial.println( String("Connecting to: " + String(mqttBroker) + ":" + String(mqttPort)));
    while (!mqttClient.connect(mqttBroker, mqttPort)) {
      delay(500);
    }
    Serial.println("MQTT Connected");
    Serial.println("Subscribing to: " + String(topicSub_ctrl));
    mqttClient.onMessage(onMqttMessage);
    mqttClient.subscribe(topicSub_ctrl);
  }

  // Write output State
  digitalWrite( CTRLPIN, curr_state);
  
  // If Switch State Changed
  if( switch_state != digitalRead(SWITCHPIN) ){
    switch_state = !switch_state;
    state_change(switch_state, 0 );
  }

  if (curr_time - prev_time > 5000) {
    prev_time = curr_time;

    float humidity = dht.readHumidity();
    float tempurature = dht.readTemperature(true);
    Serial.println("Humid: " + String(humidity));
    Serial.println("Temp (F): " + String( tempurature));

    ntpClient.update();
    msg_dht["timestamp"] = ntpClient.getEpochTime(); 
    msg_dht["temp_f"] = tempurature;
    msg_dht["humidity"] = humidity;
    
    String msg_string = JSON.stringify(msg_dht);
    mqttClient.beginMessage(topicPub_dht);
    mqttClient.print(msg_string);
    Serial.println("Sent: " + String(msg_string));
    mqttClient.endMessage();

  }
  
}

// Receive MQTT Message
void onMqttMessage(int messageSize) {
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");


  // use the Stream interface to print the contents
  int msg_char_idx = 0;
  while (mqttClient.available()) {
    msg_recv_arr[msg_char_idx++] = (char)mqttClient.read();
    Serial.print(msg_recv_arr[msg_char_idx-1]);
  }
  msg_recv_arr[msg_char_idx] = '\0';
  Serial.println();

  msg_ctrl = JSON.parse(msg_recv_arr);
  state_change((int)msg_ctrl["fan_control"], 1);
}

// Value: 0 = Off, 1 = On
// Source 0 = Manual, 1 = Server
void state_change( int value, int source ){
    curr_state = (bool)value;
    Serial.println("State Changed to " + String(value) + " By: " + String(source));
}


#include <stdio.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <Arduino_JSON.h>
#include "arduinoFFT.h"

#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0

// Select only one to be true for SAMD21. Must must be placed at the beginning before #include "SAMDTimerInterrupt.h"
#define USING_TIMER_TC3 true   // Only TC3 can be used for SAMD51
#define USING_TIMER_TC4 false  // Not to use with Servo library
#define USING_TIMER_TC5 false
#define USING_TIMER_TCC false
#define USING_TIMER_TCC1 false
#define USING_TIMER_TCC2 false  // Don't use this, can crash on some boards

#include "SAMDTimerInterrupt.h"

#define SELECTED_TIMER TIMER_TC3


const char gNumber[15] = "Gxxxx5779";  //Update with the last digits of your G-number
const char ssid[31] = "XXXX";     //Update with your WiFi SSID
const char pass[31] = "XXXX";     //Update with your WiFi password

unsigned long currMillis, prevMillis;

WiFiUDP ntpUDP;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
NTPClient ntpClient(ntpUDP);

const char mqttBroker[63] = "test.mosquitto.org";
const int mqttPort = 1883;

// The program will replace "_1234_" entry with random numbers, the G muber will be replace with gNumber entry
char mqttClienName[31] = "client_33_Gxxxx5779";
int intervalMQTT = 0;
long mqttNumberMessages = 0;
char topicPub[64]  = "";

#define TIMER_INTERVAL_US 1000  // timer interval in uSec
int analogPin = A0;
// Init selected SAMD timer
SAMDTimer ITimer(SELECTED_TIMER);

const int N_SAMPLES = 128;
const int N_AVG_FRAMES = 10;
int16_t samples[N_SAMPLES];

float fft_work_real[N_SAMPLES];
float fft_work_imag[N_SAMPLES];
ArduinoFFT<float> FFT = ArduinoFFT<float>(fft_work_real, fft_work_imag, (uint16_t)N_SAMPLES, 1E6/((float)TIMER_INTERVAL_US));

float * fft_mag;
float fft_mag_avg[N_SAMPLES];
float peak_freq;
float peak_mag;

JSONVar msg;

volatile int interupt_samples_rem = 0;
void TimerHandler() {
  if(interupt_samples_rem > 0){
    samples[N_SAMPLES-interupt_samples_rem] = (int16_t)analogRead(analogPin) - 512; // Sample and correct for DC offset
    interupt_samples_rem--;
  }
};

void setup() {
  Serial.begin(115200);  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(analogPin, INPUT);

  

  sprintf(topicPub, "%s%s%s", "gmu/ece508/", gNumber, "/adc_data");
  
  prevMillis = millis();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {        
    Serial.println("Attempting to connect to WiFi...");
    delay(250);
  }
  Serial.println("Connected to Wifi after: " + String(millis()-prevMillis) + "ms");

  ntpClient.begin();
  msg["timestamp"] = ntpClient.getEpochTime(); 
  msg["source"] = "Nano33IoT";
  msg["peak_freq"] = peak_freq;
  msg["peak_mag"] = peak_mag;

   // Interval in microseconds
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_US, TimerHandler)) {
  }else{
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
  }
  digitalWrite(LED_BUILTIN, 1);
}

void loop() {

  currMillis = millis();

  mqttClient.poll();
  if (!mqttClient.connected()) {
    Serial.println( String("Connecting to: " + String(mqttBroker) + ":" + String(mqttPort)));
    while (!mqttClient.connect(mqttBroker, mqttPort)) {
      delay(500);
    }
    Serial.println("MQTT Connected");
  }

  if (currMillis - prevMillis > 3000) {

    prevMillis = currMillis;
    

    for( int frame_idx = 0 ; frame_idx < N_AVG_FRAMES ; frame_idx++ ){
      sample_adc( samples, N_SAMPLES );
      fft_mag = compute_fft( samples, N_SAMPLES );
      average_fft(fft_mag, fft_mag_avg, N_SAMPLES, N_AVG_FRAMES);
    }

    FFT.majorPeak(fft_mag_avg, N_SAMPLES, 1E6/((float)TIMER_INTERVAL_US), &peak_freq, &peak_mag);
    reset_fft( fft_work_real, fft_work_imag, fft_mag, fft_mag_avg, N_SAMPLES);
    
    ntpClient.update();
    msg["timestamp"] = ntpClient.getEpochTime();
    msg["peak_freq"] = peak_freq;
    msg["peak_mag"] = peak_mag;
    
    String msg_string = JSON.stringify(msg);
    mqttClient.beginMessage(topicPub);
    mqttClient.print(msg_string);
    Serial.println("Sent: " + String(msg_string));
    //int bytes_sent = mqttClient.write((uint8_t*)out_msg, N_SAMPLES*2);
    //Serial.println("Sent " + String(bytes_sent) + " bytes");
    mqttClient.endMessage();
  }
  
};

void sample_adc( int16_t * samples, int n_samples ){
  interupt_samples_rem = n_samples;
  while(interupt_samples_rem){ };
  return;
};

float * compute_fft( int16_t * samples, int n_samples ){
  // Convert data to float
  for( int sample_idx = 0 ; sample_idx < n_samples ; sample_idx++ ){
    fft_work_real[sample_idx] = (float)(samples[sample_idx]);
    fft_work_imag[sample_idx] = (float)(0); // Real sampling
  }
  // Compute FFT
  FFT.compute(FFTDirection::Forward);
  
  // Compute complex mag - Stored in fft_work_real
  FFT.complexToMagnitude();
  
  return fft_work_real;
};

void average_fft( float * fft_mag, float * fft_mag_avg, int n_samples, int n_frames){
  for( int sample_idx = 0 ; sample_idx < n_samples ; sample_idx++ ){
    fft_mag_avg[sample_idx] += fft_mag[sample_idx]/((float)n_frames);
  }
};

void reset_fft( float* v0, float* v1, float* v2, float* v3, int n_samples ){
  memset( v0, 0, sizeof(float)*n_samples);
  memset( v1, 0, sizeof(float)*n_samples);
  memset( v2, 0, sizeof(float)*n_samples);
  memset( v3, 0, sizeof(float)*n_samples);
};



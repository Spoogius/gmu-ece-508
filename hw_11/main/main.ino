/**************************************************************************
Name: Stewart Schuler
Class: GMU ECE508 Internet of Things, Spring 2024
Hardware: Arduino Nano 33 IoT Dev Kit and ECE508 Breakout Board
Assignment: Homework 11 - FreeRTOS
Description: Demonstrates RTOS managing of DHT, PIR, and oLED threads.
Issues: None
**************************************************************************/

#include <stdio.h>
#include "myiot33_library.h"
#include <FreeRTOS_SAMD21.h>
#include <SimpleDHT.h>

volatile long pirTaskCounter  = 0;
volatile long dhtTaskCounter  = 0;
volatile int dispTemp = 0;

//**************************************************************************
// Type Defines and Constants
//**************************************************************************

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board
#define ERROR_LED_LIGHTUP_STATE  HIGH // the state that makes the led light up on your board, either low or high

//**************************************************************************
// global variables
//**************************************************************************
TaskHandle_t Handle_pirTask;
TaskHandle_t Handle_dhtTask;
TaskHandle_t Handle_oledTask;
TaskHandle_t Handle_monitorTask;

// PIR Function
int pir_pin = 2;
bool pir_state_val = false;

//DHT
int pinDHT22 = 5;
float tempC = 0, humidity = 0;

//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );  
}

//*****************************************************************
//OLED Display Thread
//*****************************************************************
static void thread_oled( void *pvParameters ) 
{

  char lcdBuffer[64];
  String oledline[9];
  const int oledLib = 1;

  Serial.println("Thread OLED: Started");
  // Start OLED
  iot33StartOLED(oledLib);
  oledline[1] = "G5779 FreeRTOS";
  for (int jj=2; jj<=8; jj++){ oledline[jj]=""; }
  displayTextOLED(oledline, oledLib);
  
  while(1)
  {
   
    oledline[4] = "Temp in C:" + String(tempC);
    oledline[5] = "Humidity in RH%:" + String(humidity);
    oledline[6] = "PIR detection:" + String(pir_state_val);
    pirTaskCounter = 0;
    dhtTaskCounter = 0;

    convDDHHMMSS(millis()/1000, lcdBuffer);
    oledline[8] = "Uptime: " + String(lcdBuffer);
    displayTextOLED(oledline, oledLib);
                
    myDelayMs(3000);
  }

}

static void thread_pir( void *pvParameters ) 
{
  pinMode(pir_pin, INPUT_PULLUP);
  while(1)
  {
    pir_state_val = digitalRead(pir_pin);
    pirTaskCounter++;      
    myDelayMs(500);
  }
}

static void thread_dht( void *pvParameters ) 
{
  SimpleDHT22 dht22(pinDHT22);
  int errDHT22 = SimpleDHTErrSuccess;
  while(1)
  {
    errDHT22 = dht22.read2(&tempC, &humidity, NULL);
    dhtTaskCounter++;      
    myDelayMs(2000);
  }

}

//*****************************************************************
// Task will periodically print out useful information about the tasks running
// Is a useful tool to help figure out stack sizes being used
// Run time stats are generated from all task timing collected since startup
// No easy way yet to clear the run time stats yet
//*****************************************************************
static char ptrTaskList[400]; //temporary string buffer for task stats

void taskMonitor(void *pvParameters)
{
    int x;
    int measurement;
    
    Serial.println("Task Monitor: Started");

    // run this task afew times before exiting forever
    while(1)
    {
    	myDelayMs(10000); // print every 10 seconds

		  Serial.flush();
		  Serial.println("");			 
    	Serial.println("****************************************************");
    	Serial.print("Free Heap: ");
    	Serial.print(xPortGetFreeHeapSize());
    	Serial.println(" bytes");

    	Serial.print("Min Heap: ");
    	Serial.print(xPortGetMinimumEverFreeHeapSize());
    	Serial.println(" bytes");
		  Serial.flush();

    	Serial.println("****************************************************");
    	Serial.println("Task            ABS             %Util");
    	Serial.println("****************************************************");

    	vTaskGetRunTimeStats(ptrTaskList); //save stats to char array
    	Serial.println(ptrTaskList); //prints out already formatted stats
		  Serial.flush();

	  	Serial.println("****************************************************");
	  	Serial.println("Task            State   Prio    Stack   Num     Core" );
	  	Serial.println("****************************************************");

	  	vTaskList(ptrTaskList); //save stats to char array
	  	Serial.println(ptrTaskList); //prints out already formatted stats
	  	Serial.flush();

	  	Serial.println("****************************************************");
		  Serial.println("[Stacks Free Bytes Remaining] ");

      measurement = uxTaskGetStackHighWaterMark( Handle_oledTask );
      Serial.print("Thread OLED: ");
      Serial.println(measurement);

	  	measurement = uxTaskGetStackHighWaterMark( Handle_pirTask );
	  	Serial.print("Thread PIR: ");
	  	Serial.println(measurement);

      measurement = uxTaskGetStackHighWaterMark( Handle_dhtTask );
      Serial.print("Thread DHT: ");
      Serial.println(measurement);

	  	measurement = uxTaskGetStackHighWaterMark( Handle_monitorTask );
	  	Serial.print("Monitor Stack: ");
	  	Serial.println(measurement);

	  	Serial.println("****************************************************");
	  	Serial.flush();

    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    Serial.println("Task Monitor: Deleting");
    vTaskDelete( NULL );

}


//*****************************************************************

void setup() 
{

  Serial.begin(115200); delay(500);

  //delay(1000); // prevents usb driver crash on startup, do not omit this
  //while (!Serial) ;  // Wait for serial terminal to open port before starting program


  Serial.println("");
  Serial.println("******************************");
  Serial.println("        Program start         ");
  Serial.println("******************************");
  Serial.flush();			 

  // Set the led the rtos will blink when we have a fatal rtos error
  // RTOS also Needs to know if high/low is the state that turns on the led.
  // Error Blink Codes:
  //    3 blinks - Fatal Rtos Error, something bad happened. Think really hard about what you just changed.
  //    2 blinks - Malloc Failed, Happens when you couldn't create a rtos object. 
  //               Probably ran out of heap.
  //    1 blink  - Stack overflow, Task needs more bytes defined for its stack! 
  //               Use the taskMonitor thread to help gauge how much more you need
  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

  // sets the serial port to print errors to when the rtos crashes
  // if this is not set, serial information is not printed by default
  vSetErrorSerial(&Serial);

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(thread_pir,   "Task PIR",     256, NULL, tskIDLE_PRIORITY + 4, &Handle_pirTask);
  xTaskCreate(thread_dht,   "Task DHT",     256, NULL, tskIDLE_PRIORITY + 3, &Handle_dhtTask);
  xTaskCreate(thread_oled,  "Task OLED",    256, NULL, tskIDLE_PRIORITY + 2, &Handle_oledTask);
  xTaskCreate(taskMonitor,  "Task Monitor", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_monitorTask);

  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

  // error scheduler failed to start
  // should never get here
  while(1)
  {
	  Serial.println("Scheduler Failed! \n");
	  Serial.flush();
	  delay(1000);
  }

}

//*****************************************************************
// This is now the rtos idle loop
// No rtos blocking functions allowed!
//*****************************************************************
void loop() 
{
    // Optional commands, can comment/uncomment below
    Serial.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
	  Serial.flush();			   
    delay(100); //delay is interrupt friendly, unlike vNopDelayMS
}


//*****************************************************************

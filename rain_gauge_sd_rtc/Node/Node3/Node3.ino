/*    
    Copyright 2020, Network Research Lab at the University of Toronto.

    This file is part of CottonCandy.

    CottonCandy is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CottonCandy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CottonCandy.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <LoRaMesh.h>
// #include <EbyteDeviceDriver.h>
// Uncomment the next line for using Adafruit LoRa Feather
 #include <AdafruitDeviceDriver.h>

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
//RTC_PCF8523 adalogger_rtc;


/**
 * This example demonstrates how to join a tree-based mesh network as a 
 * LoRa node and sends a random integer to the gateway upon requests.
 */ 

/**
 * Define pins for connection with Ebyte LoRa transceiver
 */
#define LORA_RX 10
#define LORA_TX 11
#define LORA_M0 2
#define LORA_M1 3
#define LORA_AUX 4

/**
 * Define pins for rain gauge 
 */
#define RAIN_GAUGE 1

const int myRTCInterruptPin = 0;

/**
 * Define variables for handling rain gauge 
 */
DateTime lastRainInterrupt;
static float daily_rain_count = 0; // each count is 0.3 mm of rain
static float interrupt_count = 0; 

LoRaMesh *manager;

DeviceDriver *myDriver;

// 2-byte long address
// For regular nodes: The first bit of the address is 0
byte myAddr[2] = {0x00, 0xA3};

/**
 * In this example, we will use union to encode and decode the long-type integer we are sending
 * in the network
 */
union LongToBytes{
  long l;
  byte b[sizeof(long)];
};


/**
 * Rain gauge interrupt service routine
 * Increments daily rain measurement
 */
void rain_gauge_interrupt() {

  
  detachInterrupt(digitalPinToInterrupt(RAIN_GAUGE));
  detachInterrupt(digitalPinToInterrupt(myRTCInterruptPin));
  
  // For debouncing
  interrupt_count = interrupt_count + 1;
  if(interrupt_count >= 3)
  {
    // Do interrupt stuff here
    Serial.println("Rain gauge pulsed");
    daily_rain_count = daily_rain_count + 1;
    interrupt_count = 0;
  }
  
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE), rain_gauge_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(myRTCInterruptPin), rtcISR, FALLING);



}
/**
 * Callback function that is called when the node receives the request from the gateway
 * and needs to reply back. Users can read sensor value, do some processing and send data back
 * to the gateway.
 * 
 * "data" points to the payload portion of the reply packet that will be sent to the gateway
 * once the function returns. Users can write their own sensor value into the "data" byte array.
 * The length of the payload can be specified by writting it to "len"
 */
void onReceiveRequest(byte **data, byte *len)
{

  // In the example, we simply send a random integer value to the gateway
  Serial.println("onReciveRequest callback");
  float rain_measurement = (float)daily_rain_count * 0.3;
  Serial.print(F("Sending number = "));
  Serial.print(rain_measurement);
  Serial.println(F(" to the gateway"));

  // Specify the length of the payload
  *len = sizeof(long);

  // Encode this long-type value into a 4-byte array
  // Note: The encoding here using C++ union is little-endian. Although the common practice in networking
  // is big-endian, to make things simple, we use the same union in both the sender(Node) and receiver(Gateway)
  // such that the number can be decoded correctly.
  union LongToBytes myConverter;
  myConverter.l = daily_rain_count;

  // Copy the encoded 4-byte array into the data (aka payload)
  memcpy(*data, myConverter.b, *len);

  // Reset rain counter
  daily_rain_count = 0;
}

void setup()
{
  Serial.begin(57600);

  // Wait for serial port to connect.
  // Please comment out this while loop when using Adafruit feather or other ATmega32u4 boards 
  // if you do not want to connect Adafruit feather to a USB port for debugging. Otherwise, the
  // feather board does not start until a serial monitor is opened.
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // In this example, we will be using an EByte E22 LoRa transceiver
  //myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, 0x09);

  // Uncomment the next line for using Adafruit LoRa Feather
   myDriver = new AdafruitDeviceDriver(myAddr);
  myDriver->init();

  // Create a LoRaMesh object
  manager = new LoRaMesh(myAddr, myDriver);

  // Set up the callback funtion
  manager->onReceiveRequest(onReceiveRequest);

//
//  // setup RTC
//  if (! adalogger_rtc.begin()) {
//    Serial.println("Couldn't find RTC");
//    Serial.flush();
//    abort();
//  }
//
//  if (! adalogger_rtc.initialized() || adalogger_rtc.lostPower()) {
//    Serial.println("RTC is NOT initialized, let's set the time!");
//    adalogger_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }
//
//  adalogger_rtc.start();
//  float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
//  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
//  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
//  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
//  int offset = round(deviation_ppm / drift_unit);
//  Serial.print("Offset is "); Serial.println(offset); // Print to control offset
//
//  lastRainInterrupt = adalogger_rtc.now();

  // Set up rain gauge interrupt 
  pinMode(RAIN_GAUGE, INPUT);
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE), rain_gauge_interrupt, FALLING);

  // Set up rtc interrupt
  pinMode(myRTCInterruptPin, INPUT_PULLUP);
}

void loop()
{
  Serial.println("Loop starts");

  // Start the node
  manager->run();
}

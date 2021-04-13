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
#include <AdafruitDeviceDriver.h>

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int chipSelect = 10;

/**
 * Define pins for connection with Ebyte LoRa transceiver
 */ 
#define LORA_RX 10
#define LORA_TX 11
#define LORA_M0 2
#define LORA_M1 3
#define LORA_AUX 4

// The time (milliseconds) between gateway requesting data
// Currently set to every 5 seconds
#define GATEWAY_REQ_TIME 15000

LoRaMesh *manager;

DeviceDriver *myDriver;

// 2-byte long address 
// For Gateway only: The first bit of the address has to be 1
byte myAddr[2] = {0x80, 0xA0};

/**
 * In this example, we will use union to encode and decode the long-type integer we are sending
 * in the network
 */
union LongToBytes{
  long l;
  byte b[sizeof(long)];
};

/**
 * Callback function that will be called when Gateway receives the reply from a node
 */
void onReciveResponse(byte *data, byte len, byte *srcAddr)
{

  // In the example, we will first print out the reply message in HEX from the node
  Serial.print(F("Gateway received a node reply from Node 0x"));
  Serial.print(srcAddr[1], HEX);
  Serial.print(srcAddr[2], HEX);

  Serial.print(F(". Data(HEX): "));

  for (int i = 0; i < len; i++)
  {
    Serial.print((data[i]), HEX);
  }
  Serial.print(".");

  // Since nodes are sending 4-byte long numbers (See example/Node/Node.ino), we can
  // convert the byte array back to a long-type integer
  if(len == 4){

    // Convert the byte array back to a long-type integer
    union LongToBytes myConverter;
    memcpy(myConverter.b, data, len);
    long value = myConverter.l;
    float rain_measurement = (float)value * 0.3;

    Serial.print(" Received rain measurement = ");
    Serial.print(rain_measurement);
    Serial.print("mm");
    Serial.println();
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      
      // Node address
      dataFile.print(srcAddr[1], HEX);
      dataFile.print(srcAddr[2], HEX);
      dataFile.print(',');

      // Sensor value
      dataFile.print(rain_measurement);
      dataFile.print(',');

      // Date
      dataFile.print(now.year(), DEC);
      dataFile.print('/');
      dataFile.print(now.month(), DEC);
      dataFile.print('/');
      dataFile.print(now.day(), DEC);
      dataFile.print(',');
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(':');
      dataFile.print(now.second(), DEC);
      dataFile.println();
      
      dataFile.println();
      dataFile.close();
      // print to the serial port too:
      Serial.print(" Saved rain reading to SD = ");
      Serial.print(rain_measurement);
      Serial.print("mm");  
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.csv");
    }
    
  }

  Serial.print('\n');

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

  // Initialize the driver
  myDriver->init();

  // Create a LoRaMesh object
  manager = new LoRaMesh(myAddr, myDriver);

  // For Gateway only: Set up the time interval between requests
  manager->setGatewayReqTime(GATEWAY_REQ_TIME);

  // Set up the callback funtion
  manager->onReceiveResponse(onReciveResponse);

  // microSD initialization
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();
  float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
  int offset = round(deviation_ppm / drift_unit);
  Serial.print("Offset is "); Serial.println(offset); // Print to control offset

  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

}

void loop()
{
  Serial.println("Loop starts");

  // Start the gateway
  manager->run();
}

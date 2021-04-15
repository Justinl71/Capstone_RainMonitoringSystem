# Rainwater Monitoring System 

There are two major components of this project. First is utilizing a LoRa mesh network with rainwater sensors attached at each node that will upload rainfall data to a gateway. This is the main focus of the project, and the framework of which was provided by a previous capstone team that designed the protocols for Multi-hop LoRa communication(CottonCandy), including the setup, recovery, and communication between the nodes and the gateway. The second component will then take this data and store it on the cloud where it can then be accessed via a web application.

## Compatible Hardware

The necessary hardware for a node:
1. WH-SP-RG Meteorological Rain Gauge
2. Adafruit Feather 32u4 RFM95 LoRa Radio
3. SMA Antenna
4. Adalogger FeatherWing 
5. 3.7V LI-POLY Battery

The necessary hardware for a gateway:
1. Adafruit Feather 32u4 RFM95 LoRa Radio
2. SMA Antenna
3. Adalogger FeatherWing 
4. MicroSD Card

## Hardware Setup

Assemble the node and gateway according to the following schematics:
![Node Schematic](https://github.com/Justinl71/Capstone_RainMonitoringSystem/blob/master/nodeDiagram.png)
![Gateway Schematic](https://github.com/Justinl71/Capstone_RainMonitoringSystem/blob/master/gatewayDiagram.png)


## Software Installation

First follow CottonCandy setup for the Adafruit Feather 32u4 device 
https://github.com/infernoDison/cottonCandy 

Install the following libraries by putting the source code into the libraries folder.
https://github.com/adafruit/RTClib
https://github.com/rocketscream/Low-Power
https://github.com/sandeepmistry/arduino-LoRa


### Set up Gateway
To program the gateway, use the vanilla ForwardEngine.cpp from cottonCandy
and the Gateway INO file from https://github.com/Justinl71/Capstone_RainMonitoringSystem/blob/master/RainDataCollection/Gateway/Gateway.ino.

Make sure to open serial monitor first as the system will not work otherwise.

### Set up Node
To program the node, go to the ForwardEngine in cottonCandy(~/Documents/Arduino/libraries/cottonCandy/ForwardEngine.cpp) and overwrite that file with our modified version at Capstone_RainMonitoringSystem/RainDataCollection/ForwardEngine.cpp

Then program each node individually, i.e. Node1.ino, Node2.ino and Node3.ino. Ensure to change the ports inside the Arduino IDE for each node.
Capstone_RainMonitoringSystem/RainDataCollection/Node/Node1/Node1.ino
Capstone_RainMonitoringSystem/RainDataCollection/Node/Node2/Node2.ino
Capstone_RainMonitoringSystem/RainDataCollection/Node/Node3/Node3.ino

Note: if using an external battery to power the node, comment out the following lines in the NodeX.ino file, otherwise the node will hang.

```cpp

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
```

### Demo 
The link below is a short demonstration of our Rainwater Monitoring System:
https://youtu.be/qaZ6czcZ2Pw


## Authors
* **Justin Leung**
* **Jeremy Lee**
* **Yongrui Zhang**

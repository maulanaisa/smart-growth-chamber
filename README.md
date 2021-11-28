# SMART GROWTH CHAMBER
Source code of Smart Plant Growth Chamber

## ESP32 Folder
Contain source codes written in arduino file format (.ino). Each source code compiled and uploaded into ESP32 Microcontroller. Written using Arduino IDE.
### Schematic
Schematic of hardware installed in Smart Growth Chamber. System runs on 3 ESP32 microcontrollers. 

ESP-A controls all actuator (heater, cooler, humidifier, minipump, fan), ESP-B is connected to all sensors but ambient temperature sensor and ESP-C controls LCD Touchscreen and reads ambient temperature sensor.

- ESP-A Schematic

![ESP-A](https://github.com/maulanaisa/smart-growth-chamber/blob/main/ESP32/ESP-A.png)

- ESP-B Schematic

![ESP-B](https://github.com/maulanaisa/smart-growth-chamber/blob/main/ESP32/ESP-B.png)

//-------------------------------------------------------------------------------------------------//

### ESP-A

#### Dependencies
```
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <RBDdimmer.h>
```
#### Highlighted pin

Heater (18), Compressor (22), Zero Cross for AC Dimmer (19), Evaporator Fan (21), Humidifier (14), Humidifier Fan (32), LED (25,26,27), Minipump (12).

//------------------------------------------------------------------------------------------------//

### ESP-B

#### Dependencies
```
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_SHT31.h>
```

#### Highlighted pin

Waterlevel Sensor (34), SHT31 (SDA:21,SCL:22), BH1750 (SDA:21,SCL:22).

//----------------------------------------------------------------------------------------------//

### ESP-C

#### Dependencies
```
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Nextion.h"
#include "max6675.h"
```

#### Highlighted pin

Ambient Temperature Sensor MAX6675 (DO:19, CS:23, CLK:5), LCD Touchscreen (TX2,RX2).


## Server Folder
Contain source code written in php. Each file acts as server-side script to receive data and images from client and/or save the data into server database.

### Script

- post_camera_back.php, post_camera_side.php, post_camera_top.php
  
  Receive images from client through HTTP post requests and save them.
  
- post_image_name.php
  
  Receive image filename from client through HTTP post and insert it into MySQL database hosted on server.

- RaspitoSQL.php
  
  Receive string data from from client through HTTP post and insert it into MySQL database hosted on server.


## Client-computer folder
Contain souce code written in python. Python scripts run as client-side program to send data/images to server.

### camera.py

When running, script is sending images to server taken from camera connected to client-computer using HTTP post request.

#### Dependencies
- pygame
- pygame.camera
- requests
- datetime
- time
- os

Depedencies can be installed using requirements_camera.txt.

### sql.py

Run script to connect client-computer as mqtt client and fetch data. Data then sent to server using HTTP post request.

#### Dependencies
- paho.mqtt.client
- requests
- time

Depedencies can be installed using requirements_sql.txt.


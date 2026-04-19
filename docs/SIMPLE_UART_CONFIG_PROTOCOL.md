# Simple UART Config Protocol

For the configuration of the device, there is a simple protocol using Asynchronous Serial, in this document,
you will find what is needed to configure the device with the serial, either manually or if you want to make a program
to do it.

## Specification

The protocol uses a very simple compact text format to set values.

Each message starts with a 2 byte break consisting of 0xAAAA and then a message length byte 3-128 is the valid range. (3
is the minimum number of characters to complete a theoretical mimimal package)

Refer to the table below for example of message construction.

| Break0 | Break1 | Length | Message String (ASCII) |
|--------|--------|--------|------------------------|
| 0xAA   | 0xAA   | 0xE    | TARGET_TEMP=10         |

The message string can contain multiple values using `;` as a seperator as shown below:

`TARGET_TEMP=10;TARGET_HUMIDITY=40`

## Parameters

Below is a list of parameters (Everything is strings, the numbers should also be ascii numbers)

| Property               | Type    | Value Range                            | Description                                                                |
|------------------------|---------|----------------------------------------|----------------------------------------------------------------------------|
| TARGET_TEMP            | Integer | 2-(Ambient Temp)                       | The target temperature of the cooling system.                              |
| TARGET_HUMIDITY        | Integer | 40-(Ambient Humidity)                  | The target humidity of the cooling system. (NOT IMPLEMENTED)               |
| MQTT_BROKER            | String  | Max 48 Characters                      | The MQTT brokers ip or address                                             |
| MQTT_PORT              | String  | Max 4 characters                       | The port used by MQTT if not default (1883)                                |
| MQTT_USER              | String  | Max 48 Characters                      | The User for the MQTT broker.                                              |
| MQTT_PASS              | String  | Max 48 Characters                      | The Password for the MQTT User.                                            |
| CMD                    | Integer | Based on Commands Table Below (Use ID) | Set this to run a command on target according the the command table below. |
| ENABLE_WIFI_NETWORKING | Bool    | 0 or 1                                 | In case of using WiFi module (ESP-AT) instead of Ethernet Module           |
| WIFI_SSID              | String  | Max 48 Characters                      | WiFi to connect to win ESP-AT Moduke                                       |
| WIFI_PASS              | String  | Max 48 Characters                      | Password for the WiFi network to connect to                                |

### Commands

The above table have a parameter called CMD, this is meant to put in commands

| Command            | ID   | Description                                                                        |
|--------------------|------|------------------------------------------------------------------------------------|
| RESTART            | 0x00 | Restarts the Device                                                                |
| RESET_SETTINGS     | 0x05 | Clears settings stored in flash.                                                   |
| SAVE_MQTT_SETTINGS | 0x06 | Saves current MQTT settings to flash to persist over restarts                      |
| SAVE_WIFI_SETTINGS | 0x07 | Saves current WiFi settings to flash to persist over restarts                      |
| ENABLE_COOLING     | 0x10 | Turns on cooling system (if off it wont run even if temp rises above the set temp) |
| DISABLE_COOLING    | 0x11 | Turns off the cooling system.                                                      |                                                      
| GET_INT_TEMP       | 0x20 | Returns the latest internal temp measurement.                                      |
| GET_INT_HUMIDITY   | 0x21 | Returns the latest internal humidity measurement.                                  |
| CONNECT_MQTT       | 0x40 | Connects to MQTT Broker with settings available.                                   |
| CONNECT_WIFI       | 0x41 | Connects to the WiFi in settings.                                                  |

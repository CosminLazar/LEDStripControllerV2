// configuration.h

#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define STRIP1CONFIG
//#define STRIP2CONFIG

//WIFI
#define STASSID "<SSID>"
#define STAPSK  "<PASSWORD>"

#if defined STRIP1CONFIG
#define STAHOSTNAME "LedStripCosmin"
#define STA_SENSORNAME "LedStripCosmin status"
#endif

#if defined STRIP2CONFIG
#define STAHOSTNAME "LedStripElla"
#define STA_SENSORNAME "LedStripElla status"
#endif

//MQTT
#define MQTT_HOST "10.0.1.251"
#define MQTT_HOST_PORT 1883

#if defined STRIP1CONFIG
#define MQTT_CLIENTID "cosminledstriptest"
#endif
#if defined STRIP2CONFIG
#define MQTT_CLIENTID "cosminledstriptest2"
#endif

#define MQTT_USERNAME "<MQTT_USERNAME>"
#define MQTT_PASSWORD "<MQTT_PASSWORD>"


#define MQTT_AVAILABILITY_ONLINE "online"
#define MQTT_AVAILABILITY_OFFLINE "offline"

#if defined STRIP1CONFIG
#define MQTT_LEDSTRIP_COMMAND "led"
#define MQTT_LEDSTRIP_STATUS "led/status"
#define MQTT_LEDSTRIP_AVAILABLITY "led/power"
#define MQTT_DEBUGMESSAGE_STATUS "led/debug"
#define MQTT_DEBUGMESSAGE_INTERVAL 1000 * 60 //60 seconds
#define MQTT_HOMEASSISTANT_AUTODISCOVERY "homeassistant/light/ledstrip/config"
#define MQTT_HOMEASSISTANT_SENSOR_AUTODISCOVERY "homeassistant/sensor/ledstrip/config"
#endif

#if defined STRIP2CONFIG
#define MQTT_LEDSTRIP_COMMAND "led2"
#define MQTT_LEDSTRIP_STATUS "led2/status"
#define MQTT_LEDSTRIP_AVAILABLITY "led2/power"
#define MQTT_DEBUGMESSAGE_STATUS "led2/debug"
#define MQTT_DEBUGMESSAGE_INTERVAL 1000 * 60 //60 seconds
#define MQTT_HOMEASSISTANT_AUTODISCOVERY "homeassistant/light/ledstripella/config"
#define MQTT_HOMEASSISTANT_SENSOR_AUTODISCOVERY "homeassistant/sensor/ledstripella/config"
#endif

//OTA
#define OTAPASSWORD "<OTA_PASSWORD>"
#define OTAPORT 8288


//LED STRIP
//#define LEDSTRIP_PIN 18
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#define LEDSTRIP_PIN 15
#define LEDSTRIP_LED_COUNT 43
#define LEDSTRIP_LED_TYPE    WS2812
#define LEDSTRIP_COLOR_ORDER GRB
#define NUM_LEDS LEDSTRIP_LED_COUNT

//TOUCH SENSOR
#define TOUCHSENSOR_PIN 4
//#define TOUCHSENSOR_PIN 19

#define EEPROM_MAXSIZE 1024
#define EEPROM_MAGICBIT_VALUE 25
#define EEPROM_MAGICBIT_POSITION 0

#endif


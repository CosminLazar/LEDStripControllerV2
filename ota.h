// ota.h

#ifndef _OTA_h
#define _OTA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#include <ArduinoOTA.h>

void setupOTA(const char *HOSTNAME, const uint16_t OTA_PORT, const char *OTA_PASSWORD);
void loopOTA();
#endif


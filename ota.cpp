// 
// 
// 

#include "ota.h"
//#include <InternalStorageESP.h>
//#include <ArduinoOTA/ArduinoOTA.h>
#include <ArduinoOTA.h>

void setupOTA(const char * hostname, const uint16_t port, const char * password)
{
	/*setup OTA*/
	ArduinoOTA.setHostname(hostname);
	ArduinoOTA.setPort(port);
	ArduinoOTA.setPassword(password);

	ArduinoOTA.onStart([]() { Serial.println("OTA: Starting"); });
	ArduinoOTA.onEnd([]() { Serial.println("\nOTA: End"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("OTA: Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.print("OTA: Error[%u]: " + error);
		if (error == OTA_AUTH_ERROR)
			Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR)
			Serial.println("End Failed");
	});

	ArduinoOTA.begin();
}

void loopOTA()
{
	ArduinoOTA.handle();
}

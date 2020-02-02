// MQTTHelper.h

#ifndef _MQTTHELPER_h
#define _MQTTHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "configuration.h"
#include <ESP8266WiFi.h>
#include "WiFiClient.h"
#include "PubSubClient.h"
typedef std::function<void(void)> cb_onConnect;

// NEED TO CHANGE MQTT_MAX_PACKET_SIZE IN PubSubClient.h to 1024 or higher 

class MQTTHelperClass
{
protected:
	WiFiClient espClient;
	PubSubClient mqttClient;
	long lastReconnectAttempt;
	cb_onConnect onConnectedCallback;
public:
	MQTTHelperClass() : mqttClient(espClient), onConnectedCallback(NULL) {}
	bool setup();
	void onMessage(std::function<void(char*, uint8_t*, unsigned int)> callback);
	void onConnected(cb_onConnect callback);
	void loop();
	void publish(const char * topic, const char * data);
	void publishRetained(const char * topic, const char * data);
	bool subscribe(const char* topic);
};

extern MQTTHelperClass MQTTHelper;

#endif


// 
// 
// 

#include "MQTTHelper.h"

bool MQTTHelperClass::setup()
{
	IPAddress mqttHostIP;
	if (!mqttHostIP.fromString(MQTT_HOST)) {
		Serial.println("Invalid mqtt host IP address");
		return false;
	}

	this->mqttClient.setServer(mqttHostIP, MQTT_HOST_PORT);

	auto connected = this->mqttClient.connect(
		MQTT_CLIENTID, 
		MQTT_USERNAME, 
		MQTT_PASSWORD,
		MQTT_LEDSTRIP_AVAILABLITY,
		MQTTQOS0,
		true,
		MQTT_AVAILABILITY_OFFLINE);

	if (connected) {
		this->mqttClient.subscribe(MQTT_LEDSTRIP_COMMAND);
		Serial.println("MQTT connected");

		if (this->onConnectedCallback != NULL) {
			this->onConnectedCallback();
		}
	}
	else {
		Serial.println("MQTT could not connect");
	}

	return connected;
}

void MQTTHelperClass::onMessage(std::function<void(char*, uint8_t*, unsigned int)> callback)
{
	this->mqttClient.setCallback(callback);
}

void MQTTHelperClass::onConnected(cb_onConnect callback)
{
	this->onConnectedCallback = callback;
}

void MQTTHelperClass::loop()
{
	boolean mqttConnected = this->mqttClient.connected();

	if (mqttConnected)
	{
		this->mqttClient.loop();
	}
	else
	{
		long now = millis();
		if (now - lastReconnectAttempt > 5000) {
			Serial.print("Retrying MQTT connection");
			this->setup();
			lastReconnectAttempt = now;
		}
	}

}

void MQTTHelperClass::publish(const char * topic, const char * data)
{
	this->mqttClient.publish(topic, data);
	
}

void MQTTHelperClass::publishRetained(const char * topic, const char * data)
{
	this->mqttClient.publish(topic, data, true);
}

bool MQTTHelperClass::subscribe(const char * topic)
{
	auto subscribed = this->mqttClient.subscribe(topic);
	return subscribed;
}


MQTTHelperClass MQTTHelper;


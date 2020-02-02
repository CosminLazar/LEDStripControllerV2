#include <AceButton.h>
#include "configuration.h"
#include "LedState.h"
#include "LedController.h"
#include <ArduinoJson.h>
#include "MQTTHelper.h"
#include <EEPROM.h>
#include "ota.h"
#include <map>

#define UPDATES_PER_SECOND 50


//using namespace ace_button;
ace_button::AceButton button;
std::map<LedEffect, const char*> supportedEffects;


void modifyState(std::function<LedStateClass(LedStateClass)> mutator)
{
	auto currentState = LedController.getState();
	auto newState = mutator(currentState);

	LedController.setState(newState);

	reportState();

	//save state
	EEPROM.write(EEPROM_MAGICBIT_POSITION, EEPROM_MAGICBIT_VALUE);
	EEPROM.put(EEPROM_MAGICBIT_POSITION + 1, newState);
	EEPROM.commit();
}

void callback(char* topic, byte* payload, unsigned int length) {
	char s[length];
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
		s[i] = payload[i];
	}

	StaticJsonDocument<512> doc;
	DeserializationError error = deserializeJson(doc, s);
	if (error)
	{
		Serial.println("Some json error");

		return;
	}

	modifyState([&doc](LedStateClass state) {
		jsonToState(doc, state);
		return state;
	});
}

void handleEvent(ace_button::AceButton*  button, uint8_t eventType, uint8_t buttonState)
{
	switch (eventType)
	{
	case ace_button::AceButton::kEventClicked:
		//toggle power
		modifyState([](LedStateClass state) {
			state.on = !state.on;
			return state;
		});
		break;
	case ace_button::AceButton::kEventDoubleClicked:
		//full power		
		modifyState([](LedStateClass state) {
			state.on = true;
			state.brightness = 255;
			state.effect = LedEffect::Static;
			state.color.r = 255;
			state.color.g = 255;
			state.color.b = 255;
			return state;
		});
		break;
	case ace_button::AceButton::kEventLongPressed:
	case ace_button::AceButton::kEventRepeatPressed:
		//increase brightness
		modifyState([](LedStateClass state) {
			state.on = true;
			state.brightness += 25;
			return state;
		});
		break;
	default:
		break;
	}
}



void setup()
{
	delay(2000);

	Serial.begin(115200);
	Serial.println("Booting");


	supportedEffects[LedEffect::Static] = "Static";
	supportedEffects[LedEffect::Wave] = "Wave";
	supportedEffects[LedEffect::Rainbow] = "Rainbow";
	supportedEffects[LedEffect::Breathe] = "Breathe";
	supportedEffects[LedEffect::BlendWave] = "BlendWave";
	supportedEffects[LedEffect::Confetti] = "Confetti";


	EEPROM.begin(EEPROM_MAXSIZE);

	/** Touch sensor **/
	pinMode(TOUCHSENSOR_PIN, INPUT);
	button.init(TOUCHSENSOR_PIN, LOW);
	ace_button::ButtonConfig* buttonConfig = button.getButtonConfig();
	buttonConfig->setEventHandler(handleEvent);
	buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
	buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureDoubleClick);
	buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureLongPress);
	buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureRepeatPress);
	buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
	buttonConfig->setClickDelay(499);
	buttonConfig->setLongPressDelay(500);
	buttonConfig->setRepeatPressInterval(500);


	if (WiFi.status() != WL_CONNECTED) {
		WiFi.setSleepMode(WIFI_NONE_SLEEP);

		WiFi.mode(WIFI_STA);
		WiFi.setAutoConnect(true);
		WiFi.setAutoReconnect(true);

		// WiFi.setSleep(false); <- does not work :(
		//esp_wifi_set_ps(WIFI_PS_NONE); //<- disables power saving https://github.com/espressif/arduino-esp32/issues/1484
		//WiFi.setHostname(STAHOSTNAME);
		WiFi.begin(STASSID, STAPSK);

#if defined STRIP1CONFIG
		IPAddress ip(10, 0, 1, 249);
#endif
#if defined STRIP2CONFIG
		IPAddress ip(10, 0, 1, 242);
#endif
		IPAddress gateway(10, 0, 1, 1);
		IPAddress subnet(255, 255, 255, 0);
		WiFi.config(ip, gateway, subnet);

		while (WiFi.waitForConnectResult() != WL_CONNECTED) {
			Serial.println("Connection Failed! Rebooting...");
			delay(5000);
			ESP.restart();
		}

		Serial.println("Ready");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	modifyState([](LedStateClass state) {
		//read saved state if any, otherwise run with the defaults
		if (EEPROM.read(EEPROM_MAGICBIT_POSITION) == EEPROM_MAGICBIT_VALUE)
		{
			EEPROM.get(EEPROM_MAGICBIT_POSITION + 1, state);
		}

		return state;
	});

	LedController.init();

	setupOTA(STAHOSTNAME, OTAPORT, OTAPASSWORD);
	setupMQTT();
}

void setupMQTT()
{
	MQTTHelper.onMessage(callback);
	MQTTHelper.onConnected(onMQTTConnected);
	MQTTHelper.setup();
}

void onMQTTConnected()
{
	publishCapabilitiesForDiscovery();
	yield();
	MQTTHelper.publishRetained(MQTT_LEDSTRIP_AVAILABLITY, MQTT_AVAILABILITY_ONLINE);
	yield();
	reportState();
}

void reportState()
{
	StaticJsonDocument<256> doc;
	LedStateClass  currentState = LedController.getState();
	stateToJson(doc, currentState);

	char output[256];
	serializeJson(doc, output);
	MQTTHelper.publish(MQTT_LEDSTRIP_STATUS, output);
}

void stateToJson(JsonDocument & doc, LedStateClass & state)
{
	doc["state"] = state.on ? "ON" : "OFF";
	doc["brightness"] = state.brightness;
	doc["color"]["r"] = state.color.r;
	doc["color"]["g"] = state.color.g;
	doc["color"]["b"] = state.color.b;
	doc["effect"] = ledEffectToString(state.effect);
}

void jsonToState(JsonDocument & doc, LedStateClass & state)
{
	if (doc.containsKey("state")) {
		const char* stateValue = doc["state"];
		state.on = strcmp(stateValue, "ON") == 0;
	}

	if (doc.containsKey("brightness"))
		state.brightness = doc["brightness"];

	if (doc.containsKey("color"))
	{
		state.color.r = doc["color"]["r"];
		state.color.g = doc["color"]["g"];
		state.color.b = doc["color"]["b"];
	}

	if (doc.containsKey("effect")) {
		const char* effect = doc["effect"].as<char *>();
		state.effect = stringToLedEffect(effect);
	}
}

LedEffect stringToLedEffect(const char * str)
{
	for (const auto& item : supportedEffects)
	{
		if (strcmp(str, item.second) == 0)
			return item.first;
	}

	return LedEffect::Static;
}

const char * ledEffectToString(LedEffect ledEffect)
{
	return supportedEffects[ledEffect];
}

void publishCapabilitiesForDiscovery()
{
	publishLedStripCapabilities();
	publishSensorCapabilities();
}

void publishLedStripCapabilities()
{
	Serial.println("MQTT: Annoucing to Home Assistant");
	const uint16_t maxMessageSize = 512;
	StaticJsonDocument<maxMessageSize> doc;

	doc["name"] = STAHOSTNAME;
	doc["unique_id"] = WiFi.macAddress();
	doc["schema"] = "json";
	doc["command_topic"] = MQTT_LEDSTRIP_COMMAND;
	doc["state_topic"] = MQTT_LEDSTRIP_STATUS;
	doc["availability_topic"] = MQTT_LEDSTRIP_AVAILABLITY;
	doc["brightness"] = true;
	doc["rgb"] = true;
	doc["effect"] = true;


	JsonArray effects = doc.createNestedArray("effect_list");
	for (const auto& item : supportedEffects)
	{
		effects.add(item.second);
	}

	auto device = doc.createNestedObject("device");
	device["name"] = STAHOSTNAME;
	device["manufacturer"] = "Cosmin Lazar";
	auto deviceIds = device.createNestedArray("identifiers");
	deviceIds.add(WiFi.macAddress());

	char output[maxMessageSize];
	serializeJson(doc, output);

	MQTTHelper.publishRetained(MQTT_HOMEASSISTANT_AUTODISCOVERY, output);
}

void publishSensorCapabilities()
{
	const uint16_t maxMessageSize = 512;
	StaticJsonDocument<maxMessageSize> doc;

	doc["name"] = STA_SENSORNAME;
	doc["unique_id"] = WiFi.macAddress() + "_status";	
	doc["availability_topic"] = MQTT_LEDSTRIP_AVAILABLITY;	
	doc["state_topic"] = MQTT_DEBUGMESSAGE_STATUS;
	doc["value_template"] = "{{ value_json.RSSI }}";
	doc["unit_of_measurement"] = " ";
	auto device = doc.createNestedObject("device");
	device["name"] = STAHOSTNAME;
	device["manufacturer"] = "Cosmin Lazar";
	auto deviceIds = device.createNestedArray("identifiers");
	deviceIds.add(WiFi.macAddress());

	char output[maxMessageSize];
	serializeJson(doc, output);

	MQTTHelper.publish(MQTT_HOMEASSISTANT_SENSOR_AUTODISCOVERY, output);
}



ulong lastDebugMessage = 0;

int WifiGetRssiAsQuality(int rssi)
{
	int quality = 0;

	if (rssi <= -100) {
		quality = 0;
	}
	else if (rssi >= -50) {
		quality = 100;
	}
	else {
		quality = 2 * (rssi + 100);
	}
	return quality;
}

void loopDebug()
{
	auto now = millis();
	if (now - lastDebugMessage > MQTT_DEBUGMESSAGE_INTERVAL)
	{
		lastDebugMessage = now;

		//trial - send all data from time to time to fight the 0 QoS of PubSub
		onMQTTConnected();
		
		yield();

		const uint16_t maxMessageSize = 256;
		StaticJsonDocument<maxMessageSize> doc;
		doc["ClientName"] = STAHOSTNAME;
		doc["RestartReason"] = ESP.getResetReason();
		doc["FreeHeap"] = ESP.getFreeHeap();
		//doc["HeapFragmentation"] = ESP.getHeapFragmentation();
		//doc["MaxFreeBlockSize"] = ESP.getMaxFreeBlockSize();
		//doc["FreeContStack"] = ESP.getFreeContStack();
		//doc["IP"] = WiFi.localIP().toString().c_str();
		//doc["RSSI"] = WifiGetRssiAsQuality(WiFi.RSSI());

		char output[maxMessageSize];
		serializeJson(doc, output);

		MQTTHelper.publish(MQTT_DEBUGMESSAGE_STATUS, output);
	}
}

void loop()
{
	//Serial.print("loop");	
	loopOTA();
	MQTTHelper.loop();
	yield();
	LedController.loop();
	yield();
	button.check();
	loopDebug();
}

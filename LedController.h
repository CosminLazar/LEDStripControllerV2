// LedController.h

#ifndef _LEDCONTROLLER_h
#define _LEDCONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "configuration.h"
#include "LedState.h"
#include <FastLED.h>
#include <functional>

#define NUM_LEDS LEDSTRIP_LED_COUNT


class LedControllerClass
{
protected:
	CRGB leds[LEDSTRIP_LED_COUNT];
	LedStateClass ledState;	
	void (LedControllerClass::*loop_effect_fn)() = &LedControllerClass::effect_solid;//sooooo, cryptic
public:
	void init();
	void setState(LedStateClass state);
	LedStateClass getState();
	void loop();
private:	
	CRGB getDesiredColor();
	void effect_strip_off();
	void effect_rainbow_march();
	void effect_waveInt();
	void effect_blendwave();
	void effect_confetti();
	void effect_confetti_loop();
	void effect_confetti_config();
	void effect_breathe();
	void effect_solid();
	void fade_to_targetBrightness();
	void fadeTowardColor(const CRGB& bgColor, uint8_t fadeAmount);
	CRGB fadeTowardColor(CRGB& cur, const CRGB& target, uint8_t amount);
	void nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount);
};

extern LedControllerClass LedController;

#endif


// LedState.h

#ifndef _LEDSTATE_h
#define _LEDSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

typedef enum {
	Static = 0,
	Breathe = 1,
	Rainbow = 2,
	Wave = 3,
	BlendWave = 4,
	Confetti = 5
} LedEffect;


struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct LedStateClass
{
public:
	Color color = { 255, 255, 255 };
	uint8_t brightness = 50;
	LedEffect effect = LedEffect::Static;
	//uint16_t speed = 1000;
	bool on = false;
};

#endif


// 
// 
// 
#define MAX_INT_VALUE 65536

#include "LedController.h"
uint16_t frame = 0;			//I think I might be able to move this variable to the void loop() scope and save some CPU
uint16_t animateSpeed = 400;          	//Number of frames to increment per loop

//blenwave
CRGB clr1;
CRGB clr2;
uint8_t speed;
uint8_t loc1;
uint8_t loc2;
uint8_t ran1;
uint8_t ran2;

//confetti
uint8_t  thisfade = 8;                                        // How quickly does it fade? Lower = slower fade rate.
int       thishue = 50;                                       // Starting hue.
uint8_t   thisinc = 1;                                        // Incremental value for rotating hues
uint8_t   thissat = 100;                                      // The saturation, where 255 = brilliant colours.
uint8_t   thisbri = 255;                                      // Brightness of a sequence. Remember, max_bright is the overall limiter.
int       huediff = 256;                                      // Range of random #'s to use for hue
uint8_t thisdelay = 5;                                        // We don't need much delay (if any)
uint8_t deltahue = 10;

void LedControllerClass::init()
{
	FastLED.addLeds<LEDSTRIP_LED_TYPE, LEDSTRIP_PIN, LEDSTRIP_COLOR_ORDER>(leds, LEDSTRIP_LED_COUNT).setCorrection(TypicalLEDStrip);

	//TODO: adjust based on power supply
	// FastLED Power management set at 5V, 2000mA.
	//FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);	
	//FastLED.setMaxRefreshRate(100);
	setState(ledState);
}

void LedControllerClass::setState(LedStateClass state)
{
	ledState = state;

	//apply state
	if (!ledState.on)
	{
		loop_effect_fn = &LedControllerClass::effect_strip_off;
		return;
	}

	//FastLED.setBrightness(ledState.brightness);

	switch (ledState.effect)
	{
	case LedEffect::Breathe:
	{
		loop_effect_fn = &LedControllerClass::effect_breathe;
		break;
	}
	case LedEffect::Rainbow:
	{
		//could i  make a lambda here to prevent global states?
		thisdelay = 200;
		deltahue = 10;
		loop_effect_fn = &LedControllerClass::effect_rainbow_march;
		break;
	}
	case LedEffect::Wave: 
	{
		loop_effect_fn = &LedControllerClass::effect_waveInt;
		break;
	}
	case LedEffect::BlendWave: 
	{
		loop_effect_fn = &LedControllerClass::effect_blendwave;
		break;
	}
	case LedEffect::Confetti: 
	{
		loop_effect_fn = &LedControllerClass::effect_confetti;
		break;
	}
	case LedEffect::Static:
	default:
	{
		loop_effect_fn = &LedControllerClass::effect_solid;
		break;
	}
	}
}

LedStateClass LedControllerClass::getState()
{
	return ledState;
}

void LedControllerClass::loop()
{

	//effect_rainbow_march(200, 10); //nice, keep

	//CHSV x = rgb2hsv_approximate(CRGB(ledState.color.r, ledState.color.g, ledState.color.b));		//nice keep
	//effect_waveInt(frame, x.hue);

	//effect_blendwave(); //ok, keep

	/*
	effect_confetti_config();                                                 // Check the demo loop for changes to the variables.
	EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
		effect_confetti();
	}
	*/

	//effect_breathe();
	//effect_solid();

	//loop the selected effect function
	(this->*loop_effect_fn)();
	FastLED.show();

	frame += animateSpeed;
}

void LedControllerClass::effect_rainbow_march() {     // The fill_rainbow call doesn't support brightness levels.

	uint8_t thishue = millis()*(255 - thisdelay) / 255;             // To change the rate, add a beat or something to the result. 'thisdelay' must be a fixed value.

  // thishue = beat8(50);                                       // This uses a FastLED sawtooth generator. Again, the '50' should not change on the fly.
  // thishue = beatsin8(50,0,255);                              // This can change speeds on the fly. You can also add these to each other.

	fill_rainbow(leds, NUM_LEDS, thishue, deltahue);            // Use FastLED's fill_rainbow routine.
	fade_to_targetBrightness();

} // rainbow_march()

//***************************   Wave [Integer Math]  *******************************
// unadulterated sine wave.  
// Uses FastLED sin16() and no float math for efficiency. 
// Since im stuck with integer values, exponential wave-forming is not possible (unless i'm wrong???)
// Color is determined by "hue"
//***********************************************************************************
void LedControllerClass::effect_waveInt() {
	CHSV desiredHsv = rgb2hsv_approximate(getDesiredColor());

	FastLED.clear();
	uint8_t value;
	for (uint8_t i = 0;i < NUM_LEDS;i++)
	{
		value = (sin16(frame + ((MAX_INT_VALUE / NUM_LEDS)*i)) + (MAX_INT_VALUE / 2)) / 256;
		if (value >= 0) {
			leds[i] += CHSV(desiredHsv.hue, 255, value);
		}
	}

	fade_to_targetBrightness();
}

void LedControllerClass::effect_blendwave() {

	speed = beatsin8(6, 0, 255);

	clr1 = blend(CHSV(beatsin8(3, 0, 255), 255, 255), CHSV(beatsin8(4, 0, 255), 255, 255), speed);
	clr2 = blend(CHSV(beatsin8(4, 0, 255), 255, 255), CHSV(beatsin8(3, 0, 255), 255, 255), speed);

	loc1 = beatsin8(10, 0, NUM_LEDS - 1);

	fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
	fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS - 1, clr1);

	fade_to_targetBrightness();

} // blendwave()

void LedControllerClass::effect_confetti()
{
	effect_confetti_config();                                                 // Check the demo loop for changes to the variables.
	EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
		effect_confetti_loop();
	}

	fade_to_targetBrightness();
}

void LedControllerClass::effect_confetti_loop() 
{
	fadeToBlackBy(leds, NUM_LEDS, thisfade);                    // Low values = slower fade.
	int pos = random16(NUM_LEDS);                               // Pick an LED at random.
	leds[pos] += CHSV((thishue + random16(huediff)) / 4, thissat, thisbri);  // I use 12 bits for hue so that the hue increment isn't too quick.
	thishue = thishue + thisinc;                                // It increments here.
}

void LedControllerClass::effect_confetti_config()
{
	uint8_t secondHand = (millis() / 1000) % 15;                // IMPORTANT!!! Change '15' to a different value to change duration of the loop.
	static uint8_t lastSecond = 99;                             // Static variable, means it's only defined once. This is our 'debounce' variable.
	if (lastSecond != secondHand) {                             // Debounce to make sure we're not repeating an assignment.
		lastSecond = secondHand;
		switch (secondHand) {
		case  0: thisinc = 1; thishue = 192; thissat = 255; thisfade = 2; huediff = 256; break;  // You can change values here, one at a time , or altogether.
		case  5: thisinc = 2; thishue = 128; thisfade = 8; huediff = 64; break;
		case 10: thisinc = 1; thishue = random16(255); thisfade = 1; huediff = 16; break;      // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
		case 15: break;                                                                // Here's the matching 15 for the other one.
		}
	}
}

void LedControllerClass::effect_breathe()
{
	//respect user settings
	fill_solid(leds, NUM_LEDS, getDesiredColor());
	auto maxBrightness = ledState.brightness;

	float breath = (exp(sin(millis() / 2000.0*PI)) - 0.36787944)*108.0;

	auto mappedBrightness = map(breath, 0, 255, 0, maxBrightness);

	FastLED.setBrightness(mappedBrightness);
	//FastLED.show();
}

void LedControllerClass::effect_solid()
{
	EVERY_N_MILLISECONDS(50) {
		CRGB desiredColor = getDesiredColor();
		fadeTowardColor(desiredColor, 5);
	}
	//solid with fade transition

	fade_to_targetBrightness();	
}

void LedControllerClass::fade_to_targetBrightness()
{
	EVERY_N_MILLISECONDS(20) {
		auto currentValue = FastLED.getBrightness();
		nblendU8TowardU8(currentValue, ledState.brightness, 10);
		//currentValue gets modified by nblendU8TowardU8
		FastLED.setBrightness(currentValue);
	}
}

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
void LedControllerClass::fadeTowardColor(const CRGB& bgColor, uint8_t fadeAmount)
{
	for (uint8_t i = 0;i < NUM_LEDS;i++)
	{
		fadeTowardColor(leds[i], bgColor, fadeAmount);
	}

	//for (uint16_t i = 0; i < N; i++) {
//		fadeTowardColor(L[i], bgColor, fadeAmount);
//	}
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB LedControllerClass::fadeTowardColor(CRGB& cur, const CRGB& target, uint8_t amount)
{
	nblendU8TowardU8(cur.red, target.red, amount);
	nblendU8TowardU8(cur.green, target.green, amount);
	nblendU8TowardU8(cur.blue, target.blue, amount);
	return cur;
}

// Helper function that blends one uint8_t toward another by a given amount
void LedControllerClass::nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount)
{
	if (cur == target) return;

	if (cur < target) {
		uint8_t delta = target - cur;
		delta = scale8_video(delta, amount);
		cur += delta;
	}
	else {
		uint8_t delta = cur - target;
		delta = scale8_video(delta, amount);
		cur -= delta;
	}
}

CRGB LedControllerClass::getDesiredColor()
{
	//if its on return color otherwise black
	return CRGB(ledState.color.r, ledState.color.g, ledState.color.b);
}

void LedControllerClass::effect_strip_off()
{
	//solid fade to black
	EVERY_N_MILLISECONDS(10) {
		CRGB desiredColor = CRGB::Black;
		fadeTowardColor(desiredColor, 5);
	}
}


LedControllerClass LedController;


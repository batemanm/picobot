/**
* Runaway for Picobot2. Lights up red in the direction it is turning.
* If there is an object with 10cm then turn a random direction
* otherwise move forward.
*
* Forward motion can be interrupted.
*
**/

#include <PicoBot.h>
#include <FastLED.h>

CRGB leds[NUM_RGBLED];

void setup (){
  FastLED.addLeds<NEOPIXEL, DATA_RGBLED>(leds, NUM_RGBLED);
}

void loop (){
  // If there is an object with 10 CM
  // randomly turn left or right for a random 
  // amount of time (between 0 and 1000 ms)
  if (Picobot.getDistance () < 10) {
    int dir = random (1000);
    int time = random (1000);
    if (dir > 500) {
      Picobot.turnRight (time, 255, 255);
      leds[0] = CRGB::Red;
      leds[1] = CRGB::Black;
    } else {
      Picobot.turnLeft (time, 255, 255);
      leds[1] = CRGB::Red;
      leds[0] = CRGB::Black;

    }
  } else {
    // If there is nothing with 10 CM
    if (Picobot.finishedMovement ()) { // and we have finished our last movement
      Picobot.forward (10, 255); // then move forward
      Picobot.interruptible (); // but we can turn if we need to
      leds[0] = CRGB::Black;
      leds[1] = CRGB::Black;
    }
  }
  FastLED.show ();
}

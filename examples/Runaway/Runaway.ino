/**
*
* If there is an object with 10cm then turn a random direction
* otherwise move forward.
*
* Forward motion can be interrupted.
*
**/

#include <PicoBot.h>

PicoBot picobot = PicoBot ();

void setup (){}

void loop (){
  // deal with moving/sensors
  picobot.serviceThreads ();

  // this is our robot's behaviour
  if (picobot.getDistance () < 10) {
    int dir = random (1000);
    int time = random (1000);
    if (dir > 500) {
      picobot.turnRight (time, 255, 255);
    } else {
      picobot.turnLeft (time, 255, 255);
    }
  } else {
    if (picobot.finishedMovement ()) {
      picobot.forward (10, 255); // move forward
      picobot.interruptible (); // but we can turn if we need to
    }
  }
}

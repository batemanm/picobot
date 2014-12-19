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
  // If there is an object with 10 CM
  // randomly turn left or right for a random 
  // amount of time (between 0 and 1000 ms)
  if (picobot.getDistance () < 10) {
    int dir = random (1000);
    int time = random (1000);
    if (dir > 500) {
      picobot.turnRight (time, 255, 255);
    } else {
      picobot.turnLeft (time, 255, 255);
    }
  } else {
    // If there is nothing with 10 CM
    if (picobot.finishedMovement ()) { // and we have finished our last movement
      picobot.forward (10, 255); // then move forward
      picobot.interruptible (); // but we can turn if we need to
    }
  }
}

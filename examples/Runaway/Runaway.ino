/**
*
* If there is an object with 10cm then turn a random direction
* otherwise move forward.
*
* Forward motion can be interrupted.
*
**/

#include <PicoBot.h>

void setup (){}

void loop (){
  // If there is an object with 10 CM
  // randomly turn left or right for a random 
  // amount of time (between 0 and 1000 ms)
  if (Picobot.getDistance () < 10) {
    int dir = random (1000);
    int time = random (1000);
    if (dir > 500) {
      Picobot.turnRight (time, 255, 255);
    } else {
      Picobot.turnLeft (time, 255, 255);
    }
  } else {
    // If there is nothing with 10 CM
    if (Picobot.finishedMovement ()) { // and we have finished our last movement
      Picobot.forward (10, 255); // then move forward
      Picobot.interruptible (); // but we can turn if we need to
    }
  }
}

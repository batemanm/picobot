/**
*
* Follows a black line.
*
**/

#include <PicoBot.h>

void setup (){
// Set the light level for the line
  Picobot.setLineLevel (500);
}

void loop (){
  if (Picobot.leftLine ()) { // If the black line is on the left, turn left
    Picobot.turnLeft (10, 255, 255);
  }
  if (Picobot.rightLine ()) { // If the black line is on the right, turn right
    Picobot.turnRight (10, 255, 255);
  }
  if (Picobot.finishedMovement ()) { // and we have finished our last movement
     Picobot.forward (10, 255); // then move forward
     Picobot.interruptible (); // but we can turn if we need to
   }
}

#include <PicoBot.h>


// Initialise the robot
PicoBot::PicoBot (){
// Setup the hardware
  pinMode(LEFT_LDR_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LDR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode (MOTOR1A_PIN, OUTPUT);
  pinMode (MOTOR1B_PIN, OUTPUT);
  pinMode (MOTOR2A_PIN, OUTPUT);
  pinMode (MOTOR2B_PIN, OUTPUT);

  pinMode (TRIGGER_PIN, OUTPUT);
  pinMode (ECHO_PIN, INPUT);


  // start the random number generator
  // should really increase the the entropy with this
  randomSeed(analogRead (LEFT_LDR_PIN));
  memset (&movement, 0, sizeof (struct movementDesc));
  long lastTimeSensor = 0;
  long lastTimeMovement = 0;

// Register a timer to deal with the movements
  cli ();
  TCCR1A = 0;
  TCCR1B = (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  OCR1A = 1000; // work out a better freq later
  sei ();
}


// Allow movements to be stopped and changed
void PicoBot::interruptible () {
  cli ();
  movement.flags = movement.flags & ~LOCKED;
  sei ();
}


// turn motors off and reset the struct
// I'll get round to writing this
void PicoBot::halt (){
  cli ();
  movement.ttr = 0;
  movement.flags = 0;
  sei ();
}


// move forward for a given time and power
// ttr - time to run for this movement
// power - the speed of this movement
void PicoBot::forward (int ttr, byte power){
  cli ();
  if (!(movement.flags & LOCKED)) {
    lastTime = millis ();
    movement.leftMotor = power;
    movement.rightMotor = power;
    movement.flags = movement.flags | LEFT_MOTOR_FORWARD;
    movement.flags = movement.flags | RIGHT_MOTOR_FORWARD;
    movement.flags = movement.flags | LOCKED;
    movement.ttr = ttr;
  }
  sei ();
}

// move backwards for a given time and power
// ttr - time to run of this movement
// power - the speed of this movement
void PicoBot::back (int ttr, byte power){
  cli ();
  lastTime = millis ();
  movement.leftMotor = power;
  movement.rightMotor = power;
  movement.flags = movement.flags & ~LEFT_MOTOR_FORWARD;
  movement.flags = movement.flags & ~RIGHT_MOTOR_FORWARD;
  movement.flags = movement.flags | LOCKED;
  movement.ttr = ttr;
  sei ();
}

// Describe the turn of the robot
// ttr - time to run this movement
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
// leftDirection - 1 is forward, 0 is backwards
// rightDirection - 1 is forward, 0 is backwards
void PicoBot::turn (int ttr, byte leftPower, byte rightPower, int leftDirection, int rightDirection) {
  cli ();
  if (!(movement.flags & LOCKED)) {
    lastTime = millis ();

    movement.ttr = ttr;
    movement.leftMotor = leftPower;
    movement.rightMotor = rightPower;
    if (leftDirection) {
      movement.flags = movement.flags | LEFT_MOTOR_FORWARD;
    } else {
      movement.flags = movement.flags & ~LEFT_MOTOR_FORWARD;
    }
    if (rightDirection) {
      movement.flags = movement.flags | RIGHT_MOTOR_FORWARD;
    } else {
      movement.flags = movement.flags & ~RIGHT_MOTOR_FORWARD;
    }
    movement.flags = movement.flags | LOCKED;
  }
  sei ();
}


// Describe a left turn on the spot
// ttr - how long to run this turn for
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
void PicoBot::turnLeft (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 0, 1);
}

// Describe a right turn on the spot
// ttr - how long to run this turn for
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
void PicoBot::turnRight (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 1, 0);
}



// Performs the movement described in movement
// NOTES
//   * currently this only runs the motors at full speed.
//   * it may run the motors for too long
//
// TODO
//   * add software PWM for backwards
//
void PicoBot::performMovement (){
  int MOTOR1A = LOW;
  int MOTOR1B = LOW;
  int MOTOR2A = LOW;
  int MOTOR2B = LOW;
  if (movement.ttr >= 0) {
    if (movement.flags & LEFT_MOTOR_FORWARD){
      MOTOR1A = LOW;
      MOTOR1B = HIGH;
    } else {
      MOTOR1A = HIGH;
      MOTOR1B = LOW;
    }
    if (movement.flags & RIGHT_MOTOR_FORWARD){
      MOTOR2A = LOW;
      MOTOR2B = HIGH;
    } else {
      MOTOR2A = HIGH;
      MOTOR2B = LOW;
    }

    long currentTime = millis ();
    long difference = currentTime - lastTime;
    movement.ttr = movement.ttr - difference;
    lastTime = currentTime;
  }
  if (movement.ttr <= 0 ){ // unlock the movement since we've finished moving
    movement.flags = movement.flags & ~LOCKED;
  }
  // Hardware PWM
  if (MOTOR1B == HIGH) {
    analogWrite (MOTOR1B_PIN, movement.leftMotor);
  } else {
    digitalWrite(MOTOR1B_PIN, MOTOR1B);
  }
  digitalWrite(MOTOR1A_PIN, MOTOR1A);

  if (MOTOR2B == HIGH) {
    analogWrite (MOTOR2B_PIN, movement.rightMotor);
  } else {
    digitalWrite(MOTOR2B_PIN, MOTOR2B);
  }
  digitalWrite(MOTOR2A_PIN, MOTOR2A);
  if (movement.ttr <= 0 ){ // unlock the movement since we've finished moving
    movement.flags = movement.flags & ~LOCKED;
  }
}

// get the distance in cm
unsigned int PicoBot::getDistance () {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  unsigned int distance = duration / 29 / 2;

  return distance;
}

// Have we finished moving?
// Returns true if we are not moving
boolean PicoBot::finishedMovement () {
  boolean result = 0;
  if (movement.ttr > 0) {
    result = 0;
  } else {
    result = 1;
  }
  return result;
}


// turns the LED on pin 13 on
void PicoBot::ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

// turns the LED on pin 13 off
void PicoBot::ledOff() {
  digitalWrite(LED_PIN, LOW);
}


// Wait until we have finished moving
void PicoBot::wait () {
  while (movement.ttr > 0) {
    delay (1);
  }
}


// Update our movement using an timer interrupt
PicoBot piobot;
ISR (TIMER1_COMPA_vect) {
  picobot.ledOn ();
  unsigned long now = millis ();
  if (picobot.lastTimeMovement < now) {
    if ((now - picobot.lastTimeMovement) > 10) {
      picobot.performMovement ();
      picobot.lastTimeMovement = now;
    }
  } else { picobot.lastTimeMovement = now;}
  picobot.ledOff ();
}

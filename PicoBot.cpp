#include <PicoBot.h>


// Initialise the hardware
PicoBot::PicoBot (){
  pinMode(LEFT_LDR_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LDR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode (MOTOR1A_PIN, OUTPUT);
  pinMode (MOTOR1B_PIN, OUTPUT);
  pinMode (MOTOR2A_PIN, OUTPUT);
  pinMode (MOTOR2B_PIN, OUTPUT);

  pinMode (TRIGGER_PIN, OUTPUT);
  pinMode (ECHO_PIN, INPUT);


//  sensorCheck = Thread ();
//  sensorCheck.enabled = true;
//  sensorCheck.onRun (checkSensor);
//  sensorCheck.setInterval (30);

//  movementCheck = Thread ();
//  movementCheck.enabled = true;
//  movementCheck.onRun (performMovement);
  // May want to change this later
  // Currently check movement 100 times per second.
//  movementCheck.setInterval (10);

  // start the random number generator
  // should really increase the the entropy with this
  randomSeed(analogRead (LEFT_LDR_PIN));
  memset (&movement, 0, sizeof (struct movementDesc));
  lastTimeSensor = 0;
  lastTimeMovement = 0;
}


// Allow movements to be stopped and changed
void PicoBot::interruptible () {
  movement.flags = movement.flags & ~LOCKED;
}


// turn motors off and reset the struct
// I'll get round to writing this
void PicoBot::halt (){
  movement.ttr = 0;
  movement.flags = 0;
}


// move forward for a given time and power
// ttr - time to run for this movement
// power - the speed of this movement
void PicoBot::forward (int ttr, byte power){
  if (!(movement.flags & LOCKED)) {
    lastTime = millis ();
    movement.leftMotor = power;
    movement.rightMotor = power;
    movement.flags = movement.flags | LEFT_MOTOR_FORWARD;
    movement.flags = movement.flags | RIGHT_MOTOR_FORWARD;
    movement.flags = movement.flags | LOCKED;
    movement.ttr = ttr;
  }
}

// move forward for a given time and power
// ttr - time to run of this movement
// power - the speed of this movement
void PicoBot::back (int ttr, byte power){
  lastTime = millis ();
  movement.leftMotor = power;
  movement.rightMotor = power;
  movement.flags = movement.flags & ~LEFT_MOTOR_FORWARD;
  movement.flags = movement.flags & ~RIGHT_MOTOR_FORWARD;
  movement.flags = movement.flags | LOCKED;
  movement.ttr = ttr;
}

// Describe the turn of the robot
// ttr - time to run this movement
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
// leftDirection - 1 is forward, 0 is backwards
// rightDirection - 1 is forward, 0 is backwards
void PicoBot::turn (int ttr, byte leftPower, byte rightPower, int leftDirection, int rightDirection) {
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
//   * include pwm for the motor speeds
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
//  if (MOTOR1B == HIGH) {
//      digitalWrite(MOTOR1B_PIN, MOTOR1B);
//    SoftPWMSet (MOTOR1B_PIN, movement.leftMotor);
//  } else {
    digitalWrite (MOTOR1B_PIN, MOTOR1B);
//  }
//  if (MOTOR1A == HIGH) {
//    SoftPWMSet (MOTOR1A_PIN, movement.leftMotor);
//  } else {
    digitalWrite(MOTOR1A_PIN, MOTOR1A);
//  }

//  if (MOTOR2B == HIGH) {
//    SoftPWMSet (MOTOR2B_PIN, movement.rightMotor);
//  } else {
    digitalWrite(MOTOR2B_PIN, MOTOR2B);
//  }
//  if (MOTOR2A == HIGH) {
//    SoftPWMSet (MOTOR2A_PIN, movement.rightMotor);
//  } else {
    digitalWrite(MOTOR2A_PIN, MOTOR2A);
//    }
}

// Should be run in your loop
// It performs the servicing of the
// movement and sensor threads.
void PicoBot::serviceThreads () {
  long now = millis ();
  if ((now - lastTimeSensor) > 30) {
    checkSensor ();
    lastTimeSensor = now;
  }
  if ((now - lastTimeMovement) > 10) {
    performMovement ();
    lastTimeMovement = now;
  }
  
}

// runs a sensor sweep
void PicoBot::checkSensor (){
  //ledOn();
//  NewPing clashes with SoftPWM :-(
//  distance = sonar.ping_cm(); // Send ping, get ping time in microseconds (uS).

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  // distance as CM
  distance = duration / 29 / 2;


  leftLdr = analogRead (LEFT_LDR_PIN);
  rightLdr = analogRead (RIGHT_LDR_PIN);
//  ledOff();
}

// get the distance
unsigned int PicoBot::getDistance () {
  return distance;
}

// Have we finished moving?
// Returns true is we are not moving
boolean PicoBot::finishedMovement () {
  boolean result = 0;
  if (movement.ttr > 0) {
    result = 0;
  } else {
    result = 1;
  }
  return result;
}


void PicoBot::ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void PicoBot::ledOff() {
  digitalWrite(LED_PIN, LOW);
}


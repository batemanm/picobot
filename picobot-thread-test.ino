/**
* Test movement code for PicoBot
* This uses two pseudo-threads, the first controls gathering information from
* the sensors. This data is left in global variables which can be used anytime
*
* The second thread controls the movement of the robot. We fill in a struct 
* movementDesc which describes how the robot should move this is then executed
* by the movement thread.
*
* TODO 
*   - include PWM support for the motors
*   - 
*/
#include <NewPing.h>
#include <Thread.h>

#define LEFT_LDR_PIN A0
#define RIGHT_LDR_PIN A1

#define TRIGGER_PIN  A4
#define ECHO_PIN     A5
#define MAX_DISTANCE 200 // furthest distance for the ping sensor 


#define MOTOR1A_PIN 2
#define MOTOR1B_PIN 3
#define MOTOR2A_PIN  4
#define MOTOR2B_PIN  5

#define LED_PIN 13

#define LEFT_MOTOR_FORWARD (1 << 0)
#define RIGHT_MOTOR_FORWARD (1 << 1)
#define LOCKED (1 << 2)

struct movementDesc {
  // left motor PWM value
  byte leftMotor;
  // right motor PWM value
  byte rightMotor;
  // flags used to decide on direction
  // we only use two bits to determine the 
  // direction of the motors
  byte flags;
  // Number of ms we should maintain this speed and direction
  // this will be decremented each time round the loop movement service loop
  // 
  int ttr;
};

// The last time we looked at movement of the robot
long lastTime = 0;

// Described what we are currently doing
struct movementDesc movement;

Thread sensorCheck = Thread ();
Thread movementCheck = Thread ();

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Globals for the sensors
unsigned int distance = 0;
int leftLdr = 0;
int rightLdr = 0;

// Allow movements to be stopped and changed
void interruptible (struct movementDesc *movement) {
  movement->flags = movement->flags & ~LOCKED;
}

// Debug procedure. Describes the current movement of the robot
// msg - debug message to print out
// movement - movementDesc that describe the movement to print
void printMovementDesc (char *msg, struct movementDesc *movement) {
  Serial.print (msg);
  Serial.print (" ");
  for (int i = 0; i < 8; i ++) {
    if (movement->flags & (1 << i)) {
      Serial.print ("1");
    } else {
      Serial.print ("0");
    }
  }
  Serial.print ("  ttr ");
  Serial.println (movement->ttr);
}

// turn motors off and reset the struct
// I'll get round to writing this
void halt (){
}

// move forward for a given time and power
// ttr - time to run for this movement
// power - the speed of this movement
void forward (int ttr, byte power){
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
void back (int ttr, byte power){
  lastTime = millis ();
  movement.leftMotor = power;
  movement.rightMotor = power;
  movement.flags = movement.flags & ~LEFT_MOTOR_FORWARD;
  movement.flags = movement.flags & ~RIGHT_MOTOR_FORWARD;
  movement.flags = movement.flags | LOCKED;
  movement.ttr = ttr;
}

// Describe a turn of the robot
// I may fold the forward/back procedures into this since they could de described using this
// function too
// ttr - time to run this movement
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
// leftDirection - 1 is forward, 0 is backwards
// rightDirection - 1 is forward, 0 is backwards
void turn (int ttr, byte leftPower, byte rightPower, int leftDirection, int rightDirection) {
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
void turnLeft (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 0, 1);
}

// Describe a right turn on the spot
// ttr - how long to run this turn for
// leftPower - the speed of the left motor
// rightPower - the speed of the right motor
void turnRight (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 1, 0);
}

// Have we finished moving?
// Returns true is we are not moving
boolean finishedMovement () {
  boolean result = 0;
  if (movement.ttr > 0) {
    result = 0;
  } else {
    result = 1;
  }
  return result;
}

// Performs the movement described in movement
// NOTES
//   * currently this only runs the motors at full speed.
//   * it may run the motors for too long 
//
// TODO 
//   * include pwm for the motor speeds
void performMovement (){
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
  digitalWrite(MOTOR1B_PIN, MOTOR1B);
  digitalWrite(MOTOR1A_PIN, MOTOR1A);
  digitalWrite(MOTOR2B_PIN, MOTOR2B);
  digitalWrite(MOTOR2A_PIN, MOTOR2A);
}

// runs a sensor sweep
void checkSensor (){
  ledOn();
  distance = sonar.ping_cm(); // Send ping, get ping time in microseconds (uS).
  leftLdr = analogRead (LEFT_LDR_PIN);
  rightLdr = analogRead (RIGHT_LDR_PIN);
  ledOff();
}

void picobot_setup (){
  Serial.begin(9600);
  
  pinMode(LEFT_LDR_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LDR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode (MOTOR1A_PIN, OUTPUT);
  pinMode (MOTOR1B_PIN, OUTPUT);
  pinMode (MOTOR2A_PIN, OUTPUT);
  pinMode (MOTOR2B_PIN, OUTPUT);


  sensorCheck.enabled = true;
  sensorCheck.onRun (checkSensor);
  // NewPing Library has a limit of around 30ms between pings
  // This is 33 times per second - should be okay
  sensorCheck.setInterval (30);
  
  movementCheck.enabled = true;
  movementCheck.onRun (performMovement);
  // May want to change this later
  // Currently check movement 100 times per second.
  movementCheck.setInterval (10);
  
  // start the random number generator
  // should really increase the the entropy with this
  randomSeed(analogRead (LEFT_LDR_PIN));
  memset (&movement, 0, sizeof (struct movementDesc));

}

// initialise the robot
// Should be moved
void setup() {
  picobot_setup ();
}

// Turn the LED on pin 13 on
void ledOn()
{
  digitalWrite(LED_PIN, HIGH);
}

// Turn the LED on pin 13 off.
void ledOff()
{
  digitalWrite(LED_PIN, LOW);
}

// Run the sensor and movement threads if we need to
void seviceThreads () {
  if (sensorCheck.shouldRun ()) {
    sensorCheck.run ();
  }
  if (movementCheck.shouldRun ()) {
    movementCheck.run ();
  }
}

void loop() {
  serviceThreads ();

/*************************************************
*
* Move forward, if we are within 10cm of an object
* then turn in a random direction for up to
* 1 second. The time to turn is also random.
*
*
**************************************************/


  if (distance < 10) {
    int dir = random (1000);
    int time = random (1000);
    if (dir > 500) {
      turnRight (time, 255, 255);
    } else {
      turnLeft (time, 255, 255);
    }
  } else {
    if (finishedMovement ()) {
      forward (10, 255); // move forward
      interruptible (&movement); // but we can turn if we need to
    }
  }
}
  

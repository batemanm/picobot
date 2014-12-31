#include <PicoBot.h>

Net PicoBot::getNetwork () {
  return net;
}

PicoBot::PicoBot (){
// Setup the hardware
  pinMode(LEFT_LDR_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LDR_PIN, INPUT_PULLUP);
  pinMode(FRONT_LED_PIN, OUTPUT);
  pinMode(REAR_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);


  pinMode (MOTOR1A_PIN, OUTPUT);
  pinMode (MOTOR1B_PIN, OUTPUT);
  pinMode (MOTOR2A_PIN, OUTPUT);
  pinMode (MOTOR2B_PIN, OUTPUT);

  pinMode (TRIGGER_PIN, OUTPUT);
  pinMode (ECHO_PIN, INPUT);


  // start the random number generator
  // should really increase the the entropy with this
  randomSeed(0);
  memset (&movement, 0, sizeof (struct movementDesc));
  movement.ttr = -1;
  long lastTimeSensor = 0;
  long lastTimeMovement = 0;

  frontLEDOff ();

// Register a timer to deal with the movements
  cli ();
  TCCR1A = 0;
  TCCR1B = (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  // FIXME - this was eyeballed - need to calculate it later.
  OCR1A = 1000;
  sei ();
}

void PicoBot::interruptible () {
  cli ();
  movement.flags = movement.flags & ~LOCKED;
  sei ();
}


void PicoBot::halt (){
  cli ();
  movement.ttr = 0;
  movement.flags = 0;
  sei ();
}

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


void PicoBot::turnLeft (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 0, 1);
}

void PicoBot::turnRight (int ttr, byte leftPower, byte rightPower) {
  turn (ttr, leftPower, rightPower, 1, 0);
}

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
  if (MOTOR1A == HIGH) {
    analogWrite (MOTOR1A_PIN, movement.leftMotor);
  } else {
    digitalWrite(MOTOR1A_PIN, MOTOR1A);
  }

  if (MOTOR2B == HIGH) {
    analogWrite (MOTOR2B_PIN, movement.rightMotor);
  } else {
    digitalWrite(MOTOR2B_PIN, MOTOR2B);
  }
  if (MOTOR2A == HIGH) {
    analogWrite (MOTOR2A_PIN, movement.rightMotor);
  } else {
    digitalWrite(MOTOR2A_PIN, MOTOR2A);
  }
  if (movement.ttr <= 0 ){ // unlock the movement since we've finished moving
    movement.flags = movement.flags & ~LOCKED;
  }
}

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

boolean PicoBot::finishedMovement () {
  boolean result = 0;
  if (movement.ttr > 0) {
    result = 0;
  } else {
    result = 1;
  }
  return result;
}

void PicoBot::frontLEDOn() {
  digitalWrite(FRONT_LED_PIN, HIGH);
}

void PicoBot::frontLEDOff() {
  digitalWrite(FRONT_LED_PIN, LOW);
}

void PicoBot::rearLEDOn() {
  digitalWrite(REAR_LED_PIN, HIGH);
}

void PicoBot::rearLEDOn(byte value) {
  analogWrite(REAR_LED_PIN, value);
}

void PicoBot::rearLEDOff() {
  digitalWrite(REAR_LED_PIN, LOW);
}

void PicoBot::wait () {
  while (movement.ttr > 0) {
    delay (1);
  }
}

int PicoBot::getLeftLDR () {
  return analogRead (LEFT_LDR_PIN);
}

int PicoBot::getRightLDR () {
  return analogRead (RIGHT_LDR_PIN);
}

void PicoBot::setLEDColour (byte red, byte green, byte blue) {
  analogWrite (RED_LED_PIN, red);
  analogWrite (GREEN_LED_PIN, green);
  analogWrite (BLUE_LED_PIN, blue);
}

//extern RF24 _nrf24l01radio;
void PicoBot::startNetworking (char *networkAddress, void (*callback)(Net *frame)) {
  byte netAddress[4];
// correct the byte ordering.
  netAddress[0] = (byte)networkAddress[3];
  netAddress[1] = (byte)networkAddress[2];
  netAddress[2] = (byte)networkAddress[1];
  netAddress[3] = (byte)networkAddress[0];

  net.setup (netAddress, callback);
}

void PicoBot::updateComms () {
  net.updateComms ();
}

PicoBot Picobot = PicoBot ();
ISR (TIMER1_COMPA_vect) {
  unsigned long now = millis ();
  if (Picobot.lastTimeMovement < now) {
    if ((now - Picobot.lastTimeMovement) > 10) {
      Picobot.performMovement ();
      Picobot.lastTimeMovement = now;
    }
  } else { 
    Picobot.lastTimeMovement = now;
  }
}


#include <PicoBot.h>

#ifdef PICOBOT1
Net PicoBot::getNetwork () {
  return net;
}
#endif

PicoBot::PicoBot (){
// Setup the hardware
  pinMode(LEFT_LDR_PIN, INPUT_PULLUP);
  pinMode(RIGHT_LDR_PIN, INPUT_PULLUP);
  pinMode(FRONT_LED_PIN, OUTPUT);
#ifdef PICOBOT1
  pinMode(REAR_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
#endif

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

  lineLevel = 400;

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

// stops the flashing too
void PicoBot::halt (){
  cli ();
//  movement.flags = movement.flags & ~FLASH_REAR_LED;
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
  long currentTime = millis ();
  long difference = currentTime - lastTime;
  lastTime = currentTime;
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
    movement.ttr = movement.ttr - difference;
    lastTime = currentTime;
  }
  if (movement.ttr <= 0){ // unlock the movement since we've finished moving
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

// flash the rear LED
#ifdef PICOBOT1
  if (movement.flags & FLASH_REAR_LED) {
    if (movement.ledStateTtr <= 0) {
      movement.ledStateTtr = movement.flashTime;
      if (movement.flags & FLASH_REAR_LED_STATE) {
        movement.flags = movement.flags & ~FLASH_REAR_LED_STATE;
        digitalWrite(REAR_LED_PIN, LOW);
      } else {
        movement.flags = movement.flags | FLASH_REAR_LED_STATE;
        digitalWrite(REAR_LED_PIN, HIGH);
      }
    } else {
      movement.ledStateTtr = movement.ledStateTtr - difference;
    }
  }
#endif
}

#ifdef PICOBOT1
void PicoBot::flashRearLED (int ttr) {
  cli ();
  movement.flags = movement.flags | FLASH_REAR_LED;
  movement.ledStateTtr = ttr;
  movement.flashTime = ttr;
  sei ();
}
#endif

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

#ifdef PICOBOT1
void PicoBot::rearLEDOn() {
  digitalWrite(REAR_LED_PIN, HIGH);
}

void PicoBot::rearLEDOn(byte value) {
  analogWrite(REAR_LED_PIN, value);
}

// turns off the rear led from flashing/constant on
void PicoBot::rearLEDOff() {
  cli ();
  movement.flashTime = 0;
  movement.flags = movement.flags & ~FLASH_REAR_LED;
  sei ();
  digitalWrite(REAR_LED_PIN, LOW);
}
#endif

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

// return AVcc in mV
long PicoBot::rawBatteryLevel (){
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}


#ifdef PICOBOT1
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

void PicoBot::ping (byte *destination, byte *responses[], int length) {
  net.ping (destination, responses, length);
}
#endif

void PicoBot::setLineLevel (int level) {
  lineLevel = level;
}

int PicoBot::leftLine () {
  return (analogRead (LEFT_LINE_PIN) < lineLevel);
}
int PicoBot::rightLine () {
  return (analogRead (RIGHT_LINE_PIN) < lineLevel);
}


PicoBot Picobot = PicoBot ();
ISR (TIMER1_COMPA_vect) {
  unsigned long now = millis ();
//  if (Picobot.lastTimeMovement < now) {
//    if ((now - Picobot.lastTimeMovement) > 10) {
      Picobot.performMovement ();
      Picobot.lastTimeMovement = now;
//    }
//  } else { 
//    Picobot.lastTimeMovement = now;
//  }
}


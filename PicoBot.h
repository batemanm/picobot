/**

PicoBot.h - Library to control a 4tronix PicoBot

**/

#ifndef PICOBOT_H
#define PICOBOT_H

#include <Arduino.h>

#define LEFT_LDR_PIN A0
#define RIGHT_LDR_PIN A1

#define TRIGGER_PIN  A4
#define ECHO_PIN     A5
#define MAX_DISTANCE 200 // furthest distance for the ping sensor - FIXME this is a guess right now

#define MOTOR1A_PIN 2
#define MOTOR1B_PIN 3
#define MOTOR2A_PIN  4
#define MOTOR2B_PIN  5

#define LED_PIN 13

#define LEFT_MOTOR_FORWARD (1 << 0)
#define RIGHT_MOTOR_FORWARD (1 << 1)
#define LOCKED (1 << 2)

#ifdef __cplusplus

extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal));


// FIXME clean up the class stucture
// it just kind of evolved.
class PicoBot {
private:
  struct movementDesc {
    // left motor PWM value
    byte leftMotor;
    // right motor PWM value
    byte rightMotor;
    // flags used to decide on direction
    byte flags;
    // Number of ms we should maintain this speed and direction
    // this will be decremented each time round the loop movement service loop
    int ttr;
  };
  // The last time we looked at movement of the robot
  long lastTimeMovement;
  long lastTime;

  // Described what we are currently doing
  // FIXME 
  //   * consider if this is better as a struct or as instance members
  struct movementDesc movement;

  void turn (int ttr, byte leftPower, byte rightPower, int leftDirection, int rightDirection);
  void performMovement ();

public:
  PicoBot ();
  void interruptible ();
  void forward (int ttr, byte power);
  void halt ();
  void back (int ttr, byte power);
  void turnLeft (int ttr, byte leftPower, byte rightPower);
  void turnRight (int ttr, byte leftPower, byte rightPower);
  boolean finishedMovement ();
  void wait ();
  void ledOn();
  void ledOff();
  unsigned int getDistance ();
  int getLeftLDR ();
  int getRightLDR ();
  friend void TIMER1_COMPA_vect ();
};

extern "C" {
  extern PicoBot picobot;
}

#else

typedef
  struct PicoBot
    PicoBot;

#endif

#endif

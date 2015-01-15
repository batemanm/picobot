/**

PicoBot.h - Library to control a 4tronix PicoBot

**/

#ifndef PICOBOT_H
#define PICOBOT_H

#include <Arduino.h>
#include "Net.h"

#define LEFT_LDR_PIN A0
#define RIGHT_LDR_PIN A1

#define LEFT_LINE_PIN A6
#define RIGHT_LINE_PIN A7

#define TRIGGER_PIN  A4
#define ECHO_PIN     A5
#define MAX_DISTANCE 200 // furthest distance for the ping sensor - FIXME this is a guess right now

#define MOTOR1A_PIN 2
#define MOTOR1B_PIN 3
#define MOTOR2A_PIN 5
#define MOTOR2B_PIN 4

#define FRONT_LED_PIN 13
#define REAR_LED_PIN 6
#define RED_LED_PIN 9
#define GREEN_LED_PIN A2
#define BLUE_LED_PIN A3

#define LEFT_MOTOR_FORWARD (1 << 0)
#define RIGHT_MOTOR_FORWARD (1 << 1)
#define LOCKED (1 << 2)
#define FLASH_REAR_LED (1 << 3)
#define FLASH_REAR_LED_STATE (1 << 4)

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
    // Flash time. This is how long the LED will be on for then off for
    int flashTime;
    // Current time to run for the LED state
    int ledStateTtr;
  };
  // Line level
  int lineLevel;
  // The last time we looked at movement of the robot
  long lastTimeMovement;
  long lastTime;
  Net net;
  struct movementDesc movement;

  // Described what we are currently doing
  // FIXME 
  //   * consider if this is better as a struct or as instance members

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
  void frontLEDOn();
  void frontLEDOff();
  unsigned int getDistance ();
  int getLeftLDR ();
  int getRightLDR ();
  void startNetworking (char *networkAddress, void (*callback)(Net *frame));
  Net getNetwork ();
  void updateComms ();
  void rearLEDOn();
  void rearLEDOn(byte value);
  void rearLEDOff();
  void flashRearLED (int ttr);
  void setLEDColour (byte red, byte green, byte blue);
  void ping (byte *dest, byte *addresses[5], int length);
  void setLineLevel (int level);
  int leftLine ();
  int rightLine ();
  friend void TIMER1_COMPA_vect ();
};


extern "C" {
  extern PicoBot Picobot;
}

#else

typedef
  struct PicoBot
    PicoBot;

#endif

#endif

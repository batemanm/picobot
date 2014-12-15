/**

PicoBot.h - Library to control a 4tronix PicoBot

**/

#ifndef PICOBOT_H
#define PICOBOT_H

// #include <ArduinoThread/Thread.h>
#include <Arduino.h>

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
  long lastTimeSensor;
  long lastTime;

  // Described what we are currently doing
  struct movementDesc movement;
  unsigned int distance; // current HC-04 distance reading
  int leftLdr; // current reading from the left ldr
  int rightLdr; // current reaing from the right ldr

  void turn (int ttr, byte leftPower, byte rightPower, int leftDirection, int rightDirection);
  void performMovement ();
  void checkSensor ();
//  Thread sensorCheck;
//  Thread movementCheck;

public:
  PicoBot ();
  void interruptible ();
  void forward (int ttr, byte power);
  void halt ();
  void back (int ttr, byte power);
  void turnLeft (int ttr, byte leftPower, byte rightPower);
  void turnRight (int ttr, byte leftPower, byte rightPower);
  boolean finishedMovement ();
  void serviceThreads ();
  void ledOn();
  void ledOff();
  unsigned int getDistance ();
  int getLeftLDR ();
  int getRightLDR ();
};

#endif

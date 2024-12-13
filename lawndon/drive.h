#ifndef DRIVE_H
#define DRIVE_H

#include "controller.h"
#include <Arduino.h>
#include <Servo.h>

// ESC
extern Servo frontLeftEsc;
extern Servo frontRightEsc;
extern Servo rearLeftEsc;
extern Servo rearRightEsc;

// pin defs
#define ESC_FRONT_LEFT_POWER    2
#define ESC_FRONT_LEFT_PWM      3
#define ESC_FRONT_RIGHT_POWER   8
#define ESC_FRONT_RIGHT_PWM     9

#define ESC_REAR_LEFT_POWER     4
#define ESC_REAR_LEFT_PWM       5
#define ESC_REAR_RIGHT_POWER    10
#define ESC_REAR_RIGHT_PWM      11

// Throttle positions
#define ESC_MIN_THROTTLE        1000
#define ESC_MAX_THROTTLE        2000
#define ESC_IDLE_THROTTLE       1500

#define ESC_ARM_SIGNAL          1000
#define ESC_ARM_TIME            2000

#define ESC_DEADZONE_RANGE      100
#define ESC_FLUTTER_RANGE       10

class Drive {
public:
  // ESC motor speeds
  int driveLeftSpeed = 0;
  int driveRightSpeed = 0;

  Drive();

  virtual void setup(void);
  virtual void loop();

  virtual void calibrateEsc(Servo esc);
  virtual void armEsc(Servo esc);
  virtual void controlDriveMotor(int speed, Servo esc);
  virtual bool isDeadzone(int speed);
};

extern Drive drive;

#endif // DRIVE_H
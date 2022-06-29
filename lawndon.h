
#define LAWNDON_H

#include "lora.h"
#include "time.h"
#include <Arduino.h>

// sensors
enum {
  SEN_BAT_VOLTAGE,
  SEN_CHG_CURRENT,
  SEN_CHG_VOLTAGE,
  SEN_MOTOR_LEFT,
  SEN_MOTOR_RIGHT
};

// actuators
enum {
  ACT_EMER_SW,
  ACT_MOTOR_LEFT,
  ACT_MOTOR_RIGHT,
  ACT_MOTOR_MOW,
};

// errors
enum {
  ERR_MOTOR_LEFT,
  ERR_MOTOR_RIGHT,
  ERR_MOTOR_MOW,
  ERR_BATTERY,
  ERR_LORA_COMM,
  ERR_LORA_DATA,
  ERR_STUCK,
  ERR_ENUM_COUNT
};

// state
enum {
  STATE_OFF,
  STATE_REMOTE,
  STATE_FORWARD,
  STATE_REVERSE,
  STATE_ROLL,
  STATE_CIRCLE,
  STATE_ERROR,
  STATE_LOCATE_FIND,
  STATE_LOCATE_TRACK
};

// roll types
enum { LEFT, RIGHT };

class Lawndon {
public:
  byte stateCurrent;
  byte stateLast;
  byte stateNext;

  unsigned long stateStartTime;
  unsigned long stateEndTime;

  // wheel motor state
  int motorSpeedMaxRpm;
  int motorSpeedMaxPwm;
  float motorPowerMax;
  float motorLeftRpmCurrent;
  float motorRightRpmCurrent;
  int motorLeftSpeedSet;
  int motorRightSpeedSet;
  int motorRollTimeMax;
  int motorRollTimeMin;
  int motorReverseTime;
  long motorForwardTimeMax;
  bool motorLeftChangeDir;
  bool motorRightChangeDir;

  // mower motor state
  int motorMowRpmCounter;
  int motorMowRpmCurrent;
  boolean motorMowEnable;
  boolean motorMowForceOff;

  // lora state
  Lora lora;

  // emergency stop
  bool emerSwitch;

  Lawndon();

  virtual void setup();

  virtual void loop();

  virtual void setMotorMowRpmState(boolean motorMowRpmState);

  virtual void setMotorPwm(int pwmLeft, int pwmRight);
  virtual void setMotorMowPwm(int pwm);

  virtual void setActuator(char type, int value) {}
  virtual void setEmerSwitch();
};
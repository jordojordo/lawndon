#include "lawndon.h"
#include "motor.h"
#include "mower.h"
#include "time.h"
#include <Arduino.h>

Lawndon::Lawndon() {
  stateLast = stateCurrent = stateNext = STATE_OFF;
  stateStartTime = 0;

  motorLeftRpmCurrent = motorRightRpmCurrent = 0;
  motorLeftSpeedSet = motorRightSpeedSet = 0;
}

void Lawndon::setEmerSwitch() { setActuator(ACT_EMER_SW, emerSwitch); }

void Lawndon::setup() {
  setMotorPwm(0, 0);
  setEmerSwitch();

  stateStartTime = 0;
  Console.println("______________BEGIN______________");
}
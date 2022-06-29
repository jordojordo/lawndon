#define MOTOR_H

#include <Arduino.h>

void setMowerMotor(int pinDir, int pinPWM, int speed);
void setDriverMotor(int pinDir, int pinPWM, int speed);
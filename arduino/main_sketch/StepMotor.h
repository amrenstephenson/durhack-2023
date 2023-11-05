#ifndef STEP_MOTOR
#define STEP_MOTOR

#include <Arduino.h>

extern int motorPorts[];

void moveSteps(bool dir, int steps, byte ms);
void moveAround(bool dir, int turns, byte ms);
void moveAngle(bool dir, int angle, byte ms);
void moveOneStep(bool dir);

#endif

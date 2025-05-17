#include <stdio.h>

extern float kp;
extern float ki;
extern float kd;
extern float error;
extern float previous_error;
extern float integral;
extern float derivative;

uint8_t pid_compute(float setpoint, float measured, float dt);
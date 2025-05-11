#include <stdio.h>

typedef struct {
    float kp;
    float ki;
    float kd;
    float previous_error;
    float integral;
} pid_controller_t;

extern pid_controller_t pid;

uint8_t pid_compute(pid_controller_t *pid, float setpoint, float measured, float dt);
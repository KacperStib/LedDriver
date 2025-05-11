#include "pid.h"

pid_controller_t pid = {
    .kp = 0.5,
    .ki = 0.1,
    .kd = 0.05,
    .previous_error = 0.0,
    .integral = 0.0
};

uint8_t pid_compute(pid_controller_t *pid, float setpoint, float measured, float dt) {
    float error = setpoint - measured;
    pid->integral += error * dt;
    float derivative = (error - pid->previous_error) / dt;
    pid->previous_error = error;

    float output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    // Ograniczenie do zakresu PWM 0–255
    if (output > 255.0) output = 255.0;
    else if (output < 0.0) output = 0.0;

    return (uint8_t)output;
}
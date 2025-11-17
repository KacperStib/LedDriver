#include "pid.h"

// Zmienne regulatora
float kp = 0.02;
float ki = 0.01;
float kd = 0.005;
float error = 0;
float previous_error = 0.0;
float integral = 0.0;
float derivative = 0.0;

uint8_t pid_compute(float setpoint, float measured, float dt) {
	// Czlon P
    error = setpoint - measured;
    // Czlon I
	// Todo: antiwindup
    integral += error * dt;
    // Czlon D
    derivative = (error - previous_error) / dt;
    previous_error = error;

    float output = (kp * error) + (ki * integral) + (kd * derivative);
	
	//printf("err=%.2f, int=%.2f, der=%.2f, out=%.2f\n", error, integral, derivative, output);
	
    // Ograniczenie sterowania do zakresu PWM 8 bit 0–255
    if (output > 255.0) output = 255.0;
    else if (output < 0.0) output = 0.0;
    
    return (uint8_t)output;
}
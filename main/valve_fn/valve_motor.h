#ifndef VALVE_MOTOR_H
#define VALVE_MOTOR_H

#include <stdint.h>

typedef struct {
    const uint8_t motorIN1_PIN;
    const uint8_t motorIN2_PIN;
    const uint8_t motorEN1_PIN;
    int state;
} Motor;

void motor_init(Motor *motor);
void motor_run_clk(Motor *motor, int dutyCycle);
void motor_run_aclck(Motor *motor, int dutyCycle);
void motor_stop(Motor *motor);

#endif // VALVE_MOTOR_H

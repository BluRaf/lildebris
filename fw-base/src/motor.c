#include <stdint.h>
#include "motor.h"

int8_t motor_cur[2] = {0, 0};

void motor_rel(int8_t x, int8_t y)
{
    motor_cur[0] += x;
    motor_cur[1] += y;
}

void motor_abs(int8_t x, int8_t y)
{
    motor_cur[0] = x;
    motor_cur[1] = y;
}


#include <stdint.h>
#include "nrf52833.h"
#include "motor.h"

static enum { MOTOR_OFF, MOTOR_ON } motor_status = MOTOR_OFF;
static int period;

#define PWM         NRF_PWM3
#define PWM_CLK     1000000
#define PWM_FREQ    500

void motor_init(int m1A, int m1B, int m2A, int m2B)
{
    /* Set prescaler divisor = 16 for 1 MHz clock */
    PWM->PRESCALER = 4;     // 16 = 1 << 4

    /* Set period */
    period = PWM_CLK / PWM_FREQ;
    PWM->COUNTERTOP = period;

    /* Connect output pins */
    PWM->PSEL.OUT[0] = m1A;
    PWM->PSEL.OUT[1] = m1B;
    PWM->PSEL.OUT[2] = m2A;
    PWM->PSEL.OUT[3] = m2B;

    /* Individual duty cycles for each channel */
    PWM->DECODER = 2;

    PWM->ENABLE = 1;
}

extern int left_motor_direction;   // 1 for forward, -1 for reverse
extern int right_motor_direction;  // 1 for forward, -1 for reverse

void motor_on(int dirA, int dutyA, int dirB, int dutyB)
{
    static uint16_t s_sequence[4];  // this cannot be on the stack
                                    // because DMA can access it after
                                    // the function returns

    switch (dirA)
    {
        case MOTOR_FORWARD:
            right_motor_direction = 1;
            s_sequence[0] = (period * dutyA) / 100;
            s_sequence[1] = 0;
            break;

        case MOTOR_REVERSE:
            right_motor_direction = -1;
            s_sequence[0] = 0;
            s_sequence[1] = (period * dutyA) / 100;
            break;

        default:
            break;
    }

    switch (dirB)
    {
        case MOTOR_FORWARD:
            left_motor_direction = 1;
            s_sequence[2] = (period * dutyB) / 100;
            s_sequence[3] = 0;
            break;

        case MOTOR_REVERSE:
            left_motor_direction = -1;
            s_sequence[2] = 0;
            s_sequence[3] = (period * dutyB) / 100;
            break;

        default:
            break;
    }

    PWM->SEQ[0].PTR = (uint32_t) s_sequence;
    PWM->SEQ[0].CNT = 4;            // one value per channel
    PWM->SEQ[0].REFRESH = 0;        // continuous

    PWM->TASKS_SEQSTART[0] = 1;
    while (PWM->EVENTS_SEQSTARTED[0] == 0)
        ;

    motor_status = MOTOR_ON;

    return;
}

void motor_off(void)
{
    right_motor_direction = 0;
    left_motor_direction = 0;

    if (motor_status == MOTOR_OFF)
        return;

    PWM->TASKS_STOP = 1;
    while (PWM->EVENTS_STOPPED == 0)
        ;

    PWM->EVENTS_STOPPED = 0;

    motor_status = MOTOR_OFF;

    return;
}

#include <stdint.h>
#include "microbit.h"
#include "bsp.h"
#include "bsp2.h"
#include "cmsis_os2.h"
#include "cross_team_definitions.h"

int16_t accel_values[3]; // [0]=X, [1]=Y, [2]=Z
int16_t avg_accel_values[3]; // [0]=X, [1]=Y, [2]=Z


void calc_avg_Acc(){

    int x_avg = 0;
    int y_avg = 0;
    int z_avg = 0;

    for(int i=0;i<5;i++){
        LSM303AGR_AccReadXYZ(accel_values);

        x_avg += accel_values[0];
        y_avg += accel_values[1];
        z_avg += accel_values[2];
        osDelay(10);
    }

    avg_accel_values[0] = x_avg/5;
    avg_accel_values[1] = y_avg/5;
    avg_accel_values[2] = z_avg/5;

    printf("%d\r\n", avg_accel_values[0]);
    printf("%d\r\n", avg_accel_values[1]);
    printf("%d\r\n", avg_accel_values[2]);
    printf("\r\n");

}


int check_fwd() {

    if (avg_accel_values[1] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}


int check_left() {


    if (avg_accel_values[2] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}

int check_right() {


    if (avg_accel_values[2] < -1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}

int check_back() {


    if (avg_accel_values[1] < -1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}


int compute_direction() {
    calc_avg_Acc();
    if (check_fwd())   return FRONT;
    if (check_back())  return BACK;
    if (check_left())  return LEFT;
    if (check_right()) return RIGHT;
    return -1; //
}
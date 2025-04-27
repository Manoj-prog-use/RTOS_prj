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

    for(int i=0;i<=10;i++){
        LSM303AGR_AccReadXYZ(accel_values);

        x_avg += accel_values[0];
        y_avg += accel_values[1];
        z_avg += accel_values[2];
        osDelay(10);
    }

    avg_accel_values[0] = x_avg/10;
    avg_accel_values[1] = y_avg/10;
    avg_accel_values[2] = z_avg/10;
}


int check_fwd() {

    if (avg_accel_values[1] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}


int check_left() {


    if (avg_accel_values[1] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}

int check_right() {


    if (avg_accel_values[1] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}

int check_back() {


    if (avg_accel_values[1] > 1000) {
        return 1; // Forward detected
    } else {
        return 0; // Not forward
    }
}


int compute_direction(){
    calc_avg_Acc();
    int fwd = check_fwd();
    int right = check_right();
    int back = check_back();
    int left = check_left();
    if(fwd==1 & right==0 & left == 0 & back ==0){
        return FRONT;
    } else if (fwd==0 & right==0 & left == 0 & back ==1)
    {
        return BACK;
    }else if (fwd==0 & right==1 & left == 1 & back ==0)
    {
        return ROTATE180;
    }else if (fwd==1 & right==1 & left == 0 & back ==0)
    {
        return RIGHT;
    }else if (fwd==1 & right==0 & left == 1 & back ==0)
    {
        return LEFT;
    }
    
    
    
    



}
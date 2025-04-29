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
    int successful_reads = 0;

    for(int i=0; i<5; i++){
        // Read accelerometer values
        LSM303AGR_AccReadXYZ(accel_values);
        
        // Add to running totals
        x_avg += accel_values[0];
        y_avg += accel_values[1];
        z_avg += accel_values[2];
        
        successful_reads++;
        
        // Give other tasks a chance to run
        osDelay(20); // Increased from 10ms to 20ms for better reliability
    }

    // Calculate averages (protect against division by zero)
    if (successful_reads > 0) {
        avg_accel_values[0] = x_avg / successful_reads;
        avg_accel_values[1] = y_avg / successful_reads; 
        avg_accel_values[2] = z_avg / successful_reads;
        
        // Use a single printf to reduce UART buffer pressure
        // printf("XYZ: %d,%d,%d\r\n", avg_accel_values[0], avg_accel_values[1], avg_accel_values[2]);
    }
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
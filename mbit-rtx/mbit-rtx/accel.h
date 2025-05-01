#ifndef ACCEL_H
#define ACCEL_H

#include "cross_team_definitions.h"

void calibrate_accelerometer();
void calc_avg_Acc();
int check_fwd();
int check_left();
int check_right();
int check_back();
int compute_direction();
void display_gesture(enum GESTURE_COMMAND gesture);

#endif /* ACCEL_H */
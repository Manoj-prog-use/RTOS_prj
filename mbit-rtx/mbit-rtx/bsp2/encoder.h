#ifndef ENCODER_H
#define ENCODER_H

void encoder_init();
void update_odometry(void);
void get_odometry(char s[]);
float get_left_speed();
float get_right_speed();
float get_robot_speed(float);
float get_angular_displacement();
void set_left_speed(float);
void set_right_speed(float);

#endif /* ENCODER_H */

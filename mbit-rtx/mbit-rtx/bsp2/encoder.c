#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "encoder.h"
#include "gpio.h"
#include "bsp2.h"
#include "bsp.h"

// Encoder Counters
volatile int32_t left_encoder_count = 0;
volatile int32_t right_encoder_count = 0;

// Previous Encoder Counts
int32_t left_encoder_prev = 0;
int32_t right_encoder_prev = 0;

// Current Position and Orientation
float x_pos = 0.0;
float y_pos = 0.0;
float theta = 0.0;

// Movement Direction (Set via commands)
int left_motor_direction = 1;  // 1 for forward, -1 for reverse
int right_motor_direction = 1; // 1 for forward, -1 for reverse

// GPIO Callback Functions for Encoders
void left_encoder_callback() {
    static uint8_t left_prev_state = 0; // Static to remember previous state
    uint8_t left_current_state;

    left_current_state = (gpio_read(C11) << 1) | gpio_read(C12); // Combine pin states

    if (left_prev_state == 0) {
        if (left_current_state == 1) {
            left_encoder_count += left_motor_direction;
        } else if (left_current_state == 3) {
            left_encoder_count -= left_motor_direction;
        }
    } else if (left_prev_state == 1) {
        if (left_current_state == 3) {
            left_encoder_count += left_motor_direction;
        } else if (left_current_state == 0) {
            left_encoder_count -= left_motor_direction;
        }
    } else if (left_prev_state == 3) {
        if (left_current_state == 2) {
            left_encoder_count += left_motor_direction;
        } else if (left_current_state == 0) {
            left_encoder_count -= left_motor_direction;
        }
    } else if (left_prev_state == 2) {
        if (left_current_state == 0) {
            left_encoder_count += left_motor_direction;
        } else if (left_current_state == 3) {
            left_encoder_count -= left_motor_direction;
        }
    }

    left_prev_state = left_current_state;
}

void right_encoder_callback() {
    static uint8_t right_prev_state = 0; // Static to remember previous state
    uint8_t right_current_state;

    right_current_state = (gpio_read(C21) << 1) | gpio_read(C22); // Combine pin states

    if (right_prev_state == 0) {
        if (right_current_state == 1) {
            right_encoder_count += right_motor_direction;
        } else if (right_current_state == 3) {
            right_encoder_count -= right_motor_direction;
        }
    } else if (right_prev_state == 1) {
        if (right_current_state == 3) {
            right_encoder_count += right_motor_direction;
        } else if (right_current_state == 0) {
            right_encoder_count -= right_motor_direction;
        }
    } else if (right_prev_state == 3) {
        if (right_current_state == 2) {
            right_encoder_count += right_motor_direction;
        } else if (right_current_state == 0) {
            right_encoder_count -= right_motor_direction;
        }
    } else if (right_prev_state == 2) {
        if (right_current_state == 0) {
            right_encoder_count += right_motor_direction;
        } else if (right_current_state == 3) {
            right_encoder_count -= right_motor_direction;
        }
    }

    right_prev_state = right_current_state;
}

void encoder_init() {
    gpio_in(C11, PULLUP);
    gpio_in(C12, PULLUP);
    gpio_in(C21, PULLUP);
    gpio_in(C22, PULLUP);

    gpio_inten(C11, 1, GPIO_BOTHEDGES, left_encoder_callback); // Trigger on any edge
    gpio_inten(C12, 1, GPIO_BOTHEDGES, left_encoder_callback); // Trigger on any edge
    gpio_inten(C21, 1, GPIO_BOTHEDGES, right_encoder_callback); // Trigger on any edge
    gpio_inten(C22, 1, GPIO_BOTHEDGES, right_encoder_callback); // Trigger on any edge
}

float calculate_distance(int32_t delta_counts) {
    return (delta_counts / (float)PRR) * (M_PI * WHEEL_DIAMETER);
}

void update_odometry() {
    // Calculate encoder deltas
    int32_t left_delta = left_encoder_count - left_encoder_prev;
    int32_t right_delta = right_encoder_count - right_encoder_prev;

    // Update previous counts
    left_encoder_prev = left_encoder_count;
    right_encoder_prev = right_encoder_count;

    // Calculate distances
    float left_distance = calculate_distance(left_delta);
    float right_distance = calculate_distance(right_delta);

    // Calculate linear displacement and heading change
    float delta_d = (left_distance + right_distance) / 2.0;
    float delta_theta = (right_distance - left_distance) / WHEEL_BASE;

    // Update pose
    theta += delta_theta;
    theta = fmod(theta, 2.0 * M_PI);
    x_pos += delta_d * cos(theta);
    y_pos += delta_d * sin(theta);
}

void get_odometry(char s[]) {
    snprintf(s, 100, "P,%f,%f,%f\n", x_pos, y_pos, theta);
}

// PID Speed Control
float left_speed_setpoint = 0.0;
float right_speed_setpoint = 0.0;

float left_kp = 1.0, left_ki = 0.0, left_kd = 0.0;
float right_kp = 1.0, right_ki = 0.0, right_kd = 0.0;

float left_integral = 0.0, right_integral = 0.0;
float left_prev_error = 0.0, right_prev_error = 0.0;

float calculate_speed(int32_t delta_counts, float time_delta) {
    return calculate_distance(delta_counts) / time_delta;
}

float pid_control(float setpoint, float current_speed, float *integral, float *prev_error, float kp, float ki, float kd, float time_delta) {
    float error = setpoint - current_speed;
    *integral += error * time_delta;
    float derivative = (error - *prev_error) / time_delta;
    float output = kp * error + ki * (*integral) + kd * derivative;
    *prev_error = error;
    return output;
}

void update_speed_control(float time_delta, float *left_motor_output, float *right_motor_output) {
    float left_speed = calculate_speed(left_encoder_count - left_encoder_prev, time_delta);
    float right_speed = calculate_speed(right_encoder_count - right_encoder_prev, time_delta);

    *left_motor_output = pid_control(left_speed_setpoint, left_speed, &left_integral, &left_prev_error, left_kp, left_ki, left_kd, time_delta);
    *right_motor_output = pid_control(right_speed_setpoint, right_speed, &right_integral, &right_prev_error, right_kp, right_ki, right_kd, time_delta);
}

void set_left_speed(float speed) {
    left_speed_setpoint = speed;
}

void set_right_speed(float speed) {
    right_speed_setpoint = speed;
}

//Angular Displacement

float get_angular_displacement(){

    return (right_encoder_count - left_encoder_count) * (M_PI * WHEEL_DIAMETER / (WHEEL_BASE * PRR));
}

float get_left_speed() {
  return calculate_speed(left_encoder_count - left_encoder_prev, 0.1); // time delta of 0.1, adjust as needed.
}

float get_right_speed() {
  return calculate_speed(right_encoder_count - right_encoder_prev, 0.1); // time delta of 0.1, adjust as needed.
}

//Overall Robot Speed
float get_robot_speed(float time_delta){
    float left_speed = get_left_speed();
    float right_speed = get_right_speed();
    return (left_speed + right_speed)/2.0;
}
#include <stdint.h>
#include "microbit.h"
#include "bsp.h"
#include "bsp2.h"
#include "cmsis_os2.h"
#include "cross_team_definitions.h"

// Global variables
int16_t accel_values[3]; // [0]=X, [1]=Y, [2]=Z
int16_t avg_accel_values[3]; // [0]=X, [1]=Y, [2]=Z
int16_t base_accel_values[3] = {0, 0, 0}; // Calibration baseline
int calibrated = 0;

// Constants for better gesture detection
#define ACCEL_SAMPLES 6        // Fewer samples for more responsiveness
#define GESTURE_THRESHOLD 800  // Detection threshold
#define GESTURE_RELEASE 400    // Lower threshold for releasing a gesture
#define GESTURE_DEBOUNCE 200   // Increased debounce time
#define CALIBRATION_SAMPLES 15 // Samples for calibration

// Last detected gesture and timestamp for debouncing
static enum GESTURE_COMMAND last_gesture = -1;
static uint32_t last_gesture_time = 0;

// Moving average filter buffer
#define FILTER_SIZE 3
int16_t filter_x[FILTER_SIZE] = {0};
int16_t filter_y[FILTER_SIZE] = {0};
int16_t filter_z[FILTER_SIZE] = {0};
int filter_index = 0;

void calibrate_accelerometer() {
    int x_sum = 0, y_sum = 0, z_sum = 0;
    
    printf("Calibrating accelerometer, please keep device still...\n");
    
    // Clear filter buffers
    for (int i = 0; i < FILTER_SIZE; i++) {
        filter_x[i] = 0;
        filter_y[i] = 0;
        filter_z[i] = 0;
    }
    
    // Take multiple samples for calibration
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        LSM303AGR_AccReadXYZ(accel_values);
        x_sum += accel_values[0];
        y_sum += accel_values[1];
        z_sum += accel_values[2];
        osDelay(20);
    }
    
    // Set baseline values
    base_accel_values[0] = x_sum / CALIBRATION_SAMPLES;
    base_accel_values[1] = y_sum / CALIBRATION_SAMPLES;
    base_accel_values[2] = z_sum / CALIBRATION_SAMPLES;
    
    calibrated = 1;
    printf("Calibration complete: %d, %d, %d\n", 
           base_accel_values[0], base_accel_values[1], base_accel_values[2]);
}

// Simple moving average filter
void apply_moving_average(int16_t x, int16_t y, int16_t z) {
    // Add new values to filter
    filter_x[filter_index] = x;
    filter_y[filter_index] = y;
    filter_z[filter_index] = z;
    
    // Move to next position in circular buffer
    filter_index = (filter_index + 1) % FILTER_SIZE;
    
    // Calculate averages
    int x_sum = 0, y_sum = 0, z_sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        x_sum += filter_x[i];
        y_sum += filter_y[i];
        z_sum += filter_z[i];
    }
    
    // Store in global averages
    avg_accel_values[0] = x_sum / FILTER_SIZE;
    avg_accel_values[1] = y_sum / FILTER_SIZE;
    avg_accel_values[2] = z_sum / FILTER_SIZE;
}

void calc_avg_Acc() {
    // Auto-calibrate if not done yet
    if (!calibrated) {
        calibrate_accelerometer();
    }
    
    int x_sum = 0, y_sum = 0, z_sum = 0;
    int successful_reads = 0;

    // Take samples
    for (int i = 0; i < ACCEL_SAMPLES; i++) {
        // Read accelerometer values
        LSM303AGR_AccReadXYZ(accel_values);
        
        // Add to running totals
        x_sum += accel_values[0] - base_accel_values[0];
        y_sum += accel_values[1] - base_accel_values[1];
        z_sum += accel_values[2] - base_accel_values[2];
        
        successful_reads++;
        osDelay(5); // Quick sampling
    }

    // Calculate averages and apply moving average filter
    if (successful_reads > 0) {
        int16_t x_avg = x_sum / successful_reads;
        int16_t y_avg = y_sum / successful_reads;
        int16_t z_avg = z_sum / successful_reads;
        
        apply_moving_average(x_avg, y_avg, z_avg);
    }
}

// Helper function for debouncing gestures - ensure when gesture is -1, we reset
int is_new_gesture(enum GESTURE_COMMAND gesture) {
    uint32_t current_time = osKernelGetTickCount();
    
    // Always update last_gesture to allow proper release detection
    if (gesture == -1) {
        last_gesture = -1; // Clear last gesture when no gesture detected
        return 0;
    }
    
    // Check if it's a new gesture or if enough time has passed
    if (gesture != last_gesture && 
        (current_time - last_gesture_time) > GESTURE_DEBOUNCE) {
        last_gesture = gesture;
        last_gesture_time = current_time;
        return 1;
    }
    return 0;
}

// Display the detected gesture on the LED screen
void display_gesture(enum GESTURE_COMMAND gesture) {
    // First clear the display
    for (int r = 0; r < LED_NUM_ROWS; r++) {
        for (int c = 0; c < LED_NUM_COLS; c++) {
            led_off(r, c);
        }
    }
    
    // Then show the appropriate letter
    switch (gesture) {
        case FRONT:
            load_letter_to_framebuffer(LETTER_F);
            break;
        case BACK:
            load_letter_to_framebuffer(LETTER_B);
            break;
        case LEFT:
            load_letter_to_framebuffer(LETTER_L);
            break;
        case RIGHT:
            load_letter_to_framebuffer(LETTER_R);
            break;
        case ROTATE180:
            load_letter_to_framebuffer(LETTER_O);
            break;
        default:
            // Display already cleared
            break;
    }
}

int compute_direction() {
    calc_avg_Acc();
    
    // Find dominant axis with highest absolute value
    int16_t abs_x = abs(avg_accel_values[0]);
    int16_t abs_y = abs(avg_accel_values[1]);
    
    enum GESTURE_COMMAND detected_gesture = -1;
    
    // Only detect gesture if acceleration exceeds threshold on some axis
    if (abs_x > GESTURE_THRESHOLD || abs_y > GESTURE_THRESHOLD) {
        // Determine which axis has the strongest signal
        if (abs_y > abs_x) {
            // Y-axis dominant (forward/backward)
            if (avg_accel_values[1] > 0) {
                detected_gesture = FRONT;  // +Y is forward
            } else {
                detected_gesture = BACK;   // -Y is backward
            }
        } else {
            // X-axis dominant (left/right)
            if (avg_accel_values[0] > 0) {
                detected_gesture = RIGHT;  // +X is right
            } else {
                detected_gesture = LEFT;   // -X is left
            }
        }
        
        if (detected_gesture != -1 && is_new_gesture(detected_gesture)) {
            // Display the gesture on the LED matrix
            display_gesture(detected_gesture);
            return detected_gesture;
        }
    } else if (abs_x < GESTURE_RELEASE && abs_y < GESTURE_RELEASE) {
        // No significant movement detected, reset gesture state
        is_new_gesture(-1);
        
        // Clear display when no gesture (optional)
        // display_gesture(-1);
    }
    
    return last_gesture; // Return current ongoing gesture or -1 if none
}

// Individual gesture check functions (kept for compatibility)
int check_fwd() {
    return (avg_accel_values[1] > GESTURE_THRESHOLD) ? 1 : 0;  // +Y
}

int check_back() {
    return (avg_accel_values[1] < -GESTURE_THRESHOLD) ? 1 : 0; // -Y
}

int check_left() {
    return (avg_accel_values[0] < -GESTURE_THRESHOLD) ? 1 : 0; // -X
}

int check_right() {
    return (avg_accel_values[0] > GESTURE_THRESHOLD) ? 1 : 0;  // +X
}
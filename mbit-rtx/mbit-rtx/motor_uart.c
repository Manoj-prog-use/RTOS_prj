#include <stdint.h>
#include <string.h>
#include "microbit.h"
#include "bsp.h"
#include "bsp2.h"
#include "cmsis_os2.h"
#include "encoder.h" // Include your encoder header

#define CMD_BUFFER_SIZE 64 // Increased buffer size

// Custom function to parse a float from a string
float my_atof(const char *str) {
    float result = 0.0;
    int decimal = 0;
    float divisor = 1.0;
    int sign = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str) {
        if (*str >= '0' && *str <= '9') {
            if (decimal) {
                result += (*str - '0') / divisor;
                divisor *= 10.0;
            } else {
                result = result * 10.0 + (*str - '0');
            }
        } else if (*str == '.') {
            decimal = 1;
        } else {
            break; // Stop at non-numeric characters
        }
        str++;
    }
    return result * sign;
}

// Custom function to extract the command and values
int parse_command(char *command, char *cmd, float *value1, float *value2) {
    char *token;
    token = strtok(command, " "); // Tokenize by space

    if (token) {
        strcpy(cmd, token);
        token = strtok(NULL, " "); // Next token

        if (token) {
            *value1 = my_atof(token);
            token = strtok(NULL, " ");

            if (token) {
                *value2 = my_atof(token);
                return 3; // Command and two values
            }
            return 2; // Command and one value
        }
        return 1; // Only command
    }
    return 0; // No command
}

void process_command(char *command) {
    char cmd[10];
    float value1, value2;
    char odometry_str[100];

    int num_values = parse_command(command, cmd, &value1, &value2);

    if (num_values >= 1) {
        if (strcmp(cmd, "FF") == 0) {
            printf("Forward\n");
            motor_on(MOTOR_FORWARD, 50, MOTOR_FORWARD, 50);
        } else if (strcmp(cmd, "FR") == 0) {
            printf("Forward Right\n");
            motor_on(MOTOR_FORWARD, 50, MOTOR_REVERSE, 50);
        } else if (strcmp(cmd, "RF") == 0) {
            printf("Reverse Forward\n");
            motor_on(MOTOR_REVERSE, 50, MOTOR_FORWARD, 50);
        } else if (strcmp(cmd, "RR") == 0) {
            printf("Reverse\n");
            motor_on(MOTOR_REVERSE, 50, MOTOR_REVERSE, 50);
        } else if (strcmp(cmd, "STOP") == 0) {
            printf("Stopping\n");
            motor_off();
        } else if (strcmp(cmd, "LSPEED") == 0 && num_values == 2) {
            printf("Set Left Speed: %f\n", value1);
            set_left_speed(value1);
        } else if (strcmp(cmd, "RSPEED") == 0 && num_values == 2) {
            printf("Set Right Speed: %f\n", value1);
            set_right_speed(value1);
        } else if (strcmp(cmd, "ODOM") == 0) {
            get_odometry(odometry_str);
            printf("Odometry: %s", odometry_str);
        } else if (strcmp(cmd, "ROBSPEED") == 0) {
            float robot_speed = get_robot_speed(0.1);
            printf("Robot Speed: %f\n", robot_speed);
        } else if (strcmp(cmd, "ANGDISP") == 0) {
            float angular_displacement = get_angular_displacement();
            printf("Angular Displacement: %f\n", angular_displacement);
        } else if (strcmp(cmd, "LSPD") == 0) {
            float left_speed = get_left_speed();
            printf("Left Speed: %f\n", left_speed);
        } else if (strcmp(cmd, "RSPD") == 0) {
            float right_speed = get_right_speed();
            printf("Right Speed: %f\n", right_speed);
        } else {
            printf("Unknown command: %s\n", command);
        }
    } else {
        printf("Invalid command format: %s\n", command);
    }
}

int uart_command_task(void) {
    char cmd_buffer[CMD_BUFFER_SIZE];
    int cmd_index = 0;
    uart_puts("UART Command Mode: Send FF, FR, RF, RR, STOP, LSPEED <value>, RSPEED <value>, ODOM, ROBSPEED, ANGDISP, LSPD, RSP\n");

    while (1) {
        char c = uart_getc();
        if (c == '\n' || c == '\r') {
            cmd_buffer[cmd_index] = '\0';
            motor_init(M1A, M1B, M2A, M2B);
            process_command(cmd_buffer);
            cmd_index = 0;
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_index++] = c;
        }

        osDelay(10); // Small delay to allow command processing
    }

    return 0;
}
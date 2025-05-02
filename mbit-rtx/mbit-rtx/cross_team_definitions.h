#ifndef CROSS_TEAM_DEFINITIONS_H
#define CROSS_TEAM_DEFINITIONS_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
// #include <stdio.h>
#include "bsp.h"
 #include "bsp2.h"
#include "cmsis_os2.h"
// #include "accel.h"
#include "motor.h"



extern int OS_READY;
extern osTimerId_t led_refresh_timer;/* DEBUG CODE */
extern osTimerId_t test_timer;  // Add this with other global declarations
void board_init(void);


enum COMMAND_TYPE
{
    GESTURE,
    ACTIVATE_BOT,
    DEACTIVATE_BOT,
    RESCUE_GESTURE,
    HEARTBEAT,
    SEARCH_BEGIN


};

/* Supported Gesture Commands on this BLE*/
enum GESTURE_COMMAND
{
    FRONT,
    RIGHT,
    BACK,
    LEFT,
    NO_GESTURE,
    ROTATE180
};

// enum RESCUE_GESTURE_COMMAND
// {
//     FRONT,
//     RIGHT,
//     BACK,
//     LEFT,
//     ROTATE180
    
// };

enum DEVICE_MODE
{
    COMMANDER,
    EXPLORER,
    SAVIOR,
    DEBUG,
    DEVICE_MODE_UNDEFINED
};

extern enum DEVICE_MODE thisDeviceMode;


struct GESTURE_COMMAND_PACKET
{
    enum GESTURE_COMMAND command;
    int rpm;
};

struct RESCUE_GESTURE_COMMAND_PACKET
{
    enum GESTURE_COMMAND command;
    int rpm;
    int seconds_elapsed;
};

struct ACTIVATE_COMMAND_PACKET
{
    enum DEVICE_MODE device_mode;
};

struct DEACTIVATE_COMMAND_PACKET
{
    enum DEVICE_MODE device_mode;
};

struct PATH_INFO_DOWNLOAD_COMMAND_PACKET
{
    int dummy_variable;
};

struct HEARTBEAT_COMMAND_PACKET
{
    int heartbeat_counter;
    int seconds_elapsed;
    float imu_radial_distance;
    float encoder_distance;
    float battery_level;
    float temperature;
};

struct SEARCH_BEGIN_COMMAND_PACKET
{
    enum DEVICE_MODE device_mode;
};

    extern int seconds_elapsed;
    extern int imu_radial_distance;
    extern int encoder_distance;
    extern int battery_level;
    extern int temperature;

void communication_init(enum DEVICE_MODE _thisDeviceMode);
enum COMMAND_TYPE GetCommandType(const char buf[],int n);
void DispatchCommand(enum COMMAND_TYPE, void* data );
void DispatchGestureCommand(struct GESTURE_COMMAND_PACKET);
void DispatchHeartbeatCommand(struct HEARTBEAT_COMMAND_PACKET heartbeat_cmd);
void DispatchActivateCommand(struct ACTIVATE_COMMAND_PACKET activate_cmd);
void DispatchRescueGestureCommand(struct RESCUE_GESTURE_COMMAND_PACKET rescue_gesture_cmd);
void DispatchSearchBeginCommand(struct SEARCH_BEGIN_COMMAND_PACKET search_begin_cmd);


#define LETTER_A 0
#define LETTER_B 1
#define LETTER_C 2
#define LETTER_D 3
#define LETTER_E 4
#define LETTER_F 5
#define LETTER_G 6
#define LETTER_H 7
#define LETTER_I 8
#define LETTER_J 9
#define LETTER_K 10
#define LETTER_L 11
#define LETTER_M 12
#define LETTER_N 13
#define LETTER_O 14
#define LETTER_P 15
#define LETTER_Q 16
#define LETTER_R 17
#define LETTER_S 18
#define LETTER_T 19
#define LETTER_U 20
#define LETTER_V 21
#define LETTER_W 22
#define LETTER_X 23
#define LETTER_Y 24
#define LETTER_Z 25

extern const int letter_patterns[26][5][5];
void load_letter_to_framebuffer(int letter_index);

#endif /* CROSS_TEAM_DEFINITIONS_H */
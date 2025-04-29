#ifndef CROSS_TEAM_DEFINITIONS_H
#define CROSS_TEAM_DEFINITIONS_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
// #include <stdio.h>
#include "bsp.h"
 #include "bsp2.h"
#include "cmsis_os2.h"
#include "accel.h"
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
    PATH_INFO_DOWNLOAD,
    HEARTBEAT,


};

/* Supported Gesture Commands on this BLE*/
enum GESTURE_COMMAND
{
    FRONT,
    RIGHT,
    BACK,
    LEFT,
    ROTATE180,
    ACTIVATE_EXPLORER
    
};

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


#endif /* CROSS_TEAM_DEFINITIONS_H */
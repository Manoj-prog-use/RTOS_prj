#ifndef CROSS_TEAM_DEFINITIONS_H
#define CROSS_TEAM_DEFINITIONS_H



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
    DEBUG
};

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

#endif /* CROSS_TEAM_DEFINITIONS_H */
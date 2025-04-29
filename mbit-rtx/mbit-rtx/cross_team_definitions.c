#include "cross_team_definitions.h"

 enum DEVICE_MODE thisDeviceMode = DEVICE_MODE_UNDEFINED;
 int OS_READY = 0;
/*
Write a dedicated  thread in Explorer bot  to update these values. 
These are global variables, and will be read by separate thread in explorer to dispatch 
the heartbeat to the commander. 
*/
 int seconds_elapsed=0;
 int imu_radial_distance=0;
 int encoder_distance=0;
 int battery_level=100;
 int temperature=37;

 osTimerId_t led_refresh_timer;/* DEBUG CODE */

void board_init(void)
 {
    leds_init();
    buttons_init();
    uart_init(UART_RX, UART_TX);
    audio_init(SPEAKER, MIC, RUN_MIC);


  communication_init(COMMANDER);
  LSM303AGR_Init(I2C_SCL, I2C_SDA);
  motor_init( M1A,  M1B,  M2A,  M2B);
  motor_off();

 }


void communication_init(enum DEVICE_MODE _thisDeviceMode)
{
    thisDeviceMode = _thisDeviceMode;
}


enum COMMAND_TYPE GetCommandType(const char buf[],int n)
{
    //Command type will be stored in first byte. We'll cast it as an unsidned int lol
    if(n>0)
    {
        printf("%i \n",n);
        switch (buf[0])
        {
        case (int)(GESTURE): return GESTURE; 
        case (int)(ACTIVATE_BOT): return ACTIVATE_BOT; 
        case (int)(DEACTIVATE_BOT): return DEACTIVATE_BOT; 
        case (int)(PATH_INFO_DOWNLOAD): return PATH_INFO_DOWNLOAD; 
        case (int)(HEARTBEAT): return HEARTBEAT; 

        
        default:
            break;
        }
    }
    else
    {
        printf("%s","Message Size Zero");
        return -1;
    }
}

void DispatchCommand(enum COMMAND_TYPE _commandType, void* data )
{
    switch(_commandType)
    {
        case GESTURE:
        {
        struct GESTURE_COMMAND_PACKET* gesture_cmd = (struct GESTURE_COMMAND_PACKET*)data;
        DispatchGestureCommand(*gesture_cmd);
        break;
        }
         case HEARTBEAT:
        {
        struct HEARTBEAT_COMMAND_PACKET* heartbeat_cmd = (struct HEARTBEAT_COMMAND_PACKET*)data;
        DispatchHeartbeatCommand(*heartbeat_cmd);
        break;
        }
        // {
        // struct ACTIVATE_COMMAND_PACKET* activate_cmd = (struct ACTIVATE_COMMAND_PACKET*)data;
        // DispatchActivateCommand(*activate_cmd);
        // break;
        // }
        // case DEACTIVATE_BOT:
        // {
        // struct DEACTIVATE_COMMAND_PACKET* deactivate_cmd = (struct DEACTIVATE_COMMAND_PACKET*)data;
        // DispatchDeactivateCommand(*deactivate_cmd);
        // break;
        // }
        // case PATH_INFO_DOWNLOAD:
        // {
        // struct PATH_INFO_DOWNLOAD_COMMAND_PACKET* path_info_download_cmd = (struct PATH_INFO_DOWNLOAD_COMMAND_PACKET*)data;
        // DispatchPathInfoDownloadCommand(*path_info_download_cmd);
        // break;
        // }

        


    }
}

void DispatchGestureCommand(struct GESTURE_COMMAND_PACKET gesture_cmd)
{
    // Create a buffer to hold the serialized data
    // Buffer size needs to be at least as large as the struct
    char buffer[sizeof(struct GESTURE_COMMAND_PACKET) + 1];  // +1 for command type
    
    // First byte indicates the command type (GESTURE)
    buffer[0] = GESTURE;  // Set first byte to identify packet type
    
    // Serialize the struct into the buffer (starting at position 1)
    memcpy(&buffer[1], &gesture_cmd, sizeof(struct GESTURE_COMMAND_PACKET));
    
    // Calculate the total size of the message
    unsigned int message_size = sizeof(struct GESTURE_COMMAND_PACKET) + 1;
    
    // Send the packet over radio
    radio_send(buffer, message_size);
    
    // Optional: Display debug info
    printf("Sent gesture command: Type=%d, RPM=%d\n", 
           gesture_cmd.command, gesture_cmd.rpm);
    
    // Optional: Visual feedback that command was sent
    // led_blink(1, 1);
}

void DispatchHeartbeatCommand(struct HEARTBEAT_COMMAND_PACKET heartbeat_cmd)
{
    char buffer[sizeof(struct HEARTBEAT_COMMAND_PACKET) + 1];  // +1 for command type
    
    // First byte indicates the command type (HEARTBEAT)
    buffer[0] = HEARTBEAT;  // Set first byte to identify packet type
    
    // Serialize the struct into the buffer (starting at position 1)
    memcpy(&buffer[1], &heartbeat_cmd, sizeof(struct HEARTBEAT_COMMAND_PACKET));
    
    // Calculate the total size of the message
    unsigned int message_size = sizeof(struct HEARTBEAT_COMMAND_PACKET) + 1;
    
    // Send the packet over radio
    radio_send(buffer, message_size);

    printf("Sent heartbeat command: Seconds Elapsed=%d, IMU Radial Distance=%d, Encoder Distance=%d, Battery Level=%d, Temperature=%d\n", 
           heartbeat_cmd.seconds_elapsed, heartbeat_cmd.imu_radial_distance, heartbeat_cmd.encoder_distance, 
           heartbeat_cmd.battery_level, heartbeat_cmd.temperature);
    
}

void DispatchActivateCommand(struct ACTIVATE_COMMAND_PACKET activate_cmd)
{
    char buffer[sizeof(struct ACTIVATE_COMMAND_PACKET) + 1];  // +1 for command type
    
    // First byte indicates the command type (ACTIVATE_BOT)
    buffer[0] = ACTIVATE_BOT;  // Set first byte to identify packet type
    
    // Serialize the struct into the buffer (starting at position 1)
    memcpy(&buffer[1], &activate_cmd, sizeof(struct ACTIVATE_COMMAND_PACKET));
    
    // Calculate the total size of the message
    unsigned int message_size = sizeof(struct ACTIVATE_COMMAND_PACKET) + 1;
    
    // Send the packet over radio
    radio_send(buffer, message_size);
}
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "bsp.h"
 #include "bsp2.h"
#include "cmsis_os2.h"
#include "cross_team_definitions.h"
#include "accel.h"
#include "motor.h"

/* DEBUG CODE */
/* DEBUG CODE */
/* DEBUG CODE */
osTimerId_t led_refresh_timer;/* DEBUG CODE */
osTimerId_t test_timer;  // Add this with other global declarations

/* DEBUG CODE */
/* DEBUG CODE */

/*COMMON*/
    int seconds_elapsed=0;
    int imu_radial_distance=0;
    int encoder_distance=0;
    int battery_level=100;
    int temperature=37;
void communication_init(enum DEVICE_MODE _thisDeviceMode);
/*COMMON*/

/*COMMANDER*/
void commander_init(void);
void commander_radio_callback(const char buf[], unsigned int n);
void CommandSenderThread(void *argument);
struct HEARTBEAT_COMMAND_PACKET parseHeartbeatPacket(const char buf[], unsigned int n);
void PushHeartbeatIntoQueue(struct HEARTBEAT_COMMAND_PACKET _packet);
/*COMMANDER*/

/*EXPLORER*/
osMessageQueueId_t gestureQueueId;
void explorer_init(void);
void explorer_radio_callback(const char buf[], unsigned int n);
void InitializeGestureQueue(void);
void GestureQueueThread(void *argument);
void ExplorerHeartbeatThread(void *argument);
void DispatchHeartbeatCommand(struct HEARTBEAT_COMMAND_PACKET heartbeat_cmd);
void ActOnGestureCommand(enum GESTURE_COMMAND gesture, int rpm);
/*EXPLORER*/

/*SAVIOR*/
void savior_init(void);
void savior_radio_callback(const char buf[], unsigned int n);
/*SAVIOR*/

/*DEBUG*/
void debug_init(void);
void DebugThread(void *argument);
/*DEBUG*/




struct GESTURE_COMMAND_PACKET command_array[1000];
enum COMMAND_TYPE GetCommandType(const char buf[],int n);
void PushGestureIntoQueue(struct GESTURE_COMMAND_PACKET _packet);
struct GESTURE_COMMAND_PACKET parseGesturePacket(const char buf[],unsigned int n);
void DispatchCommand(enum COMMAND_TYPE, void* data );
void DispatchGestureCommand(struct GESTURE_COMMAND_PACKET);
// void DispatchActivateCommand(struct ACTIVATE_COMMAND_PACKET);
// void DispatchDeactivateCommand(struct DEACTIVATE_COMMAND_PACKET);
// void DispatchPathInfoDownloadCommand(struct PATH_INFO_DOWNLOAD);






int  OS_READY = 0;
















/* To be manually changed and compiled for each microbit device
Two microbit cannot have same device MODE*/
enum DEVICE_MODE thisDeviceMode;






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

//     return;
 }

int main(void)
{

    OS_READY = 0;

    board_init();


    printf("hello, world!\n");

      osKernelInitialize();
//    osThreadNew(uart_command_task, NULL, NULL);
    if(thisDeviceMode == EXPLORER)
    {
        explorer_init();
    }
    else if(thisDeviceMode == COMMANDER)
    {
        commander_init();
          

    }
    else if(thisDeviceMode == SAVIOR)
    {
        savior_init();
    }
    else if(thisDeviceMode == DEBUG)
    {
        debug_init();
    }

    /* never returns */
    osKernelStart();

    

    return 0;
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


struct GESTURE_COMMAND_PACKET parseGesturePacket(const char buf[], unsigned int n)
{
   struct GESTURE_COMMAND_PACKET result;
   memcpy(&result, buf + 1, sizeof(struct GESTURE_COMMAND_PACKET));
   return result;
}

void PushGestureIntoQueue(struct GESTURE_COMMAND_PACKET _packet)
{
    //  Put the command into the RTOS queue
    osStatus_t status = osMessageQueuePut(gestureQueueId, &_packet, 0, 0);
    
    if (status != osOK) {
        // Failed to add to queue - handle error
        printf("Failed to enqueue gesture command\n");
    }

    //  ActOnGestureCommand(_packet.command, _packet.rpm);

        for (int r = 0; r < LED_NUM_ROWS; r++) {
        for (int c = 0; c < LED_NUM_COLS; c++) {
            led_off(r,c);
        }
    }
    int led_button_number = _packet.rpm;
    
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


void InitializeGestureQueue(void) {
    // Create a queue that can hold up to 16 gesture commands
    gestureQueueId = osMessageQueueNew(30, sizeof(struct GESTURE_COMMAND_PACKET), NULL);
    
    if (gestureQueueId == NULL) {
        // Queue creation failed - handle error
        printf("Failed to create gesture command queue\n");
    }
}

void GestureQueueThread(void *argument)
{
    OS_READY = 1;

    struct GESTURE_COMMAND_PACKET gcp;

    while (1)
    {
        // Wait forever for a new message in the queue
        osStatus_t status = osMessageQueueGet(gestureQueueId, &gcp, NULL, osWaitForever);

        if (status == osOK)
        {
            // Act on the received gesture command
            ActOnGestureCommand(gcp.command, gcp.rpm);
        }
        // Optionally handle errors here
    }
}



void ActOnGestureCommand(enum GESTURE_COMMAND gesture, int rpm)
{
    // Scale rpm (0-5) to PWM speed (0-100)
    int speed = rpm * 20;

    switch (gesture) {
        case FRONT:
            // Move forward
            motor_on(MOTOR_FORWARD, speed, MOTOR_FORWARD, speed);
            break;
        case BACK:
            // Move backward
            motor_on(MOTOR_REVERSE, speed, MOTOR_REVERSE, speed);
            break;
        case RIGHT:
            // Turn right (left wheel forward, right wheel backward)
            motor_on(MOTOR_FORWARD, speed, MOTOR_REVERSE, speed);
            break;
        case LEFT:
            // Turn left (left wheel backward, right wheel forward)
            motor_on(MOTOR_REVERSE, speed, MOTOR_FORWARD, speed);
            break;
        case ROTATE180:
            // Rotate in place (example: both wheels opposite directions)
            motor_on(MOTOR_FORWARD, speed, MOTOR_REVERSE, speed);
            // You may want to add a delay or logic to stop after 180 degrees
            break;
        default:
            // Stop motors for unknown gesture
            motor_off();
            break;
    }
}

void ExplorerHeartbeatThread(void *argument)
{
    static int heartbeat_counter = 0;
    while(1)
    {
        heartbeat_counter++;
        seconds_elapsed = osKernelGetTickCount() / osKernelGetTickFreq();
        struct HEARTBEAT_COMMAND_PACKET hcp;
        hcp.heartbeat_counter = heartbeat_counter;
        hcp.seconds_elapsed = seconds_elapsed;
        hcp.imu_radial_distance = imu_radial_distance;
        hcp.encoder_distance = encoder_distance;
        hcp.battery_level = battery_level;
        hcp.temperature = temperature;

        DispatchCommand(HEARTBEAT, (void *)&hcp);

        osDelay(1000);
    }
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

void explorer_init(void)
{
           InitializeGestureQueue();
           osThreadNew(GestureQueueThread, NULL, NULL);
           osThreadNew(ExplorerHeartbeatThread, NULL, NULL);
            radio_init(explorer_radio_callback);


};
void commander_init(void)
{
     osThreadNew(CommandSenderThread, NULL, NULL);
     led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);
         if (led_refresh_timer == NULL) {
        printf("LED timer creation failed!\n");
    }
    radio_init(commander_radio_callback);

     


};
void savior_init(void)
{
    radio_init(savior_radio_callback);
};

void explorer_radio_callback(const char buf[], unsigned int n)
{
    if(OS_READY == 1)
        {
        /*
        my_radio_callback will just push the command into a global vector. 
        Some other interrupt will come in and read this command
        */
        if(GetCommandType(buf,n)== GESTURE)
        {
            printf("%s","GESTURE COMMAND FOUND");
            struct GESTURE_COMMAND_PACKET gestureCommandPacket =  parseGesturePacket(buf,n);
            PushGestureIntoQueue(gestureCommandPacket);
        }
    }

}

void commander_radio_callback(const char buf[], unsigned int n)
{
    if(OS_READY == 1)
        {
        /*
        my_radio_callback will just push the command into a global vector. 
        Some other interrupt will come in and read this command
        */  
        if(GetCommandType(buf,n)== HEARTBEAT)
        {
            printf("%s","HEARTBEAT COMMAND FOUND\r\n");
            struct HEARTBEAT_COMMAND_PACKET heartbeatCommandPacket =  parseHeartbeatPacket(buf,n);
            PushHeartbeatIntoQueue(heartbeatCommandPacket);
        }
        }
};

struct HEARTBEAT_COMMAND_PACKET parseHeartbeatPacket(const char buf[], unsigned int n)
{
    struct HEARTBEAT_COMMAND_PACKET result;
    memcpy(&result, buf + 1, sizeof(struct HEARTBEAT_COMMAND_PACKET));
    return result;
}

void PushHeartbeatIntoQueue(struct HEARTBEAT_COMMAND_PACKET _packet)
{
    printf("Heartbead packet: %d, %d, %d, %d, %d\n", _packet.heartbeat_counter, _packet.seconds_elapsed, _packet.imu_radial_distance, _packet.encoder_distance, _packet.battery_level, _packet.temperature);
    // osMessageQueuePut(heartbeatQueueId, &_packet, 0, 0);
}

void savior_radio_callback(const char buf[], unsigned int n)
{

};


void test_timer_callback(void *argument)
{
    printf("Test timer tick\n");
}


void debug_init(void)
{
    osThreadNew(DebugThread, NULL, NULL);
    // Check LED timer creation
    led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);
    if (led_refresh_timer == NULL) {
        printf("LED timer creation failed!\n");
    }
    
    // Check test timer creation
    test_timer = osTimerNew((void *)test_timer_callback, osTimerPeriodic, NULL, NULL);
    if (test_timer == NULL) {
        printf("Test timer creation failed!\n");
    }

    
};


void CommandSenderThread(void *argument)
{  
    OS_READY = 1;
            osStatus_t status = osTimerStart(led_refresh_timer, 5);
         printf("LED timer start status: %d\n", (int)status);
    enum GESTURE_COMMAND prev_gesture = -1; // Initialize to an invalid value

    while (1)
    {
        frame_buffer[3][3] = 1;  // Turn on
        // 1. Detect current gesture
        enum GESTURE_COMMAND current_gesture = compute_direction();



        // 2. If gesture changed, send new command
        if (current_gesture != prev_gesture && current_gesture != -1)
        {
                     
         osDelay(200); 

            struct GESTURE_COMMAND_PACKET gcp;
            gcp.command = current_gesture;
            gcp.rpm = 3; // Or any speed you want

            DispatchCommand(GESTURE, (void *)&gcp);

            
            prev_gesture = current_gesture;
            
            
        }

        // 3. Wait before checking again
       
        osDelay(250); // 200 ms, adjust as needed
                 for (int r = 0; r < LED_NUM_ROWS; r++) {
            for (int c = 0; c < LED_NUM_COLS; c++) {
                frame_buffer[r][c] = 0;
            }
        }
        osDelay(250);
        fflush(stdout); 
        
        // osThreadExit();
    }

}

void DebugThread(void *argument)
{
        osStatus_t status = osTimerStart(led_refresh_timer, 5);
         printf("LED timer start status: %d\n", (int)status);
    while(1)
    {
        frame_buffer[0][0] = 1;  // Turn on
        osDelay(500);
        printf("%d\n",frame_buffer[0][0]);
        frame_buffer[0][0] = 0;
        osDelay(500);
        printf("%d\n",frame_buffer[0][0]);
        
    //     frame_buffer[0][0] = 0;  // Turn off
    //     printf("OFF\n");
    //     osDelay(1000);
    }
}
// extern void uart_command_task(void *arg);

// /* OS objects */
// // osThreadId_t tid1, tid2;

// // #define MAX_COUNT 100

// void task1(void *arg)
// {
//     uint32_t r, c;
//     uint32_t r0, c0;
//     int count;

//     printf("hello, task1!\n");

//     r = 0; c = 0;
//     r0 = 0; c0 = 1;

//     while (1)
//     {
//         for (count = 0; count < MAX_COUNT; count++)
//         {
//             frame_buffer[r][c] = 1;
//             osDelay(50);
//             frame_buffer[r][c] = 0;

//             r += r0;
//             c += c0;

//             if ((r == 0) && (c == 0))
//             {
//                 c0 = 1; r0 = 0;
//             }
//             else if ((r == 0) && ((c == (LED_NUM_COLS - 1))))
//             {
//                 c0 = 0; r0 = 1;
//             }
//             else if ((r == (LED_NUM_ROWS - 1)) && ((c == (LED_NUM_COLS - 1))))
//             {
//                 c0 = -1; r0 = 0;
//             }
//             else if ((r == (LED_NUM_ROWS - 1)) && (c == 0))
//             {
//                 c0 = 0; r0 = -1;
//             }
//         }
//     }
// }

// void task2(void *arg)
// {
//     uint32_t r, c;
//     uint32_t r0, c0;

//     printf("hello, task2!\n");

//     r = 1; c = 1;
//     r0 = 0; c0 = 1;

//     while (1)
//     {
//         frame_buffer[r][c] = 1;
//         osDelay(50);
//         frame_buffer[r][c] = 0;

//         r += r0;
//         c += c0;

//         if ((r == 1) && (c == 1))
//         {
//             c0 = 1; r0 = 0;
//         }
//         else if ((r == 1) && ((c == (LED_NUM_ROWS - 2))))
//         {
//             c0 = 0; r0 = 1;
//         }
//         else if ((r == (LED_NUM_COLS - 2)) && ((c == (LED_NUM_ROWS - 2))))
//         {
//             c0 = -1; r0 = 0;
//         }
//         else if ((r == (LED_NUM_COLS - 2)) && (c == 1))
//         {
//             c0 = 0; r0 = -1;
//         }
//     }
// }

// void task3(void *arg)
// {
//     int toggle1 = 0, toggle2 = 0;

//     printf("hello, task3!\n");

//     while (1)
//     {
//         frame_buffer[2][2] ^= 1;
//         osDelay(100);

//         if (button_get(0))
//         {
//             osDelay(25);
//             while (button_get(0))
//                 ;

//             if (toggle1)
//                 osThreadResume(tid1);
//             else
//                 osThreadSuspend(tid1);

//             toggle1 = !toggle1;
//         }

//         if (button_get(1))
//         {
//             osDelay(25);
//             while (button_get(1))
//                 ;

//             if (toggle2)
//                 osThreadResume(tid2);
//             else
//                 osThreadSuspend(tid2);

//             toggle2 = !toggle2;
//         }
//     }
// }

// void app_main(void *arg)
// {
//     osTimerId_t timer_id;
//     osThreadId_t tid;

//     /* runner tasks */
//     tid1 = osThreadNew(task1, NULL, NULL);
//     tid2 = osThreadNew(task2, NULL, NULL);

//     /* controller tasks */
//     tid = osThreadNew(task3, NULL, NULL);
//     osThreadSetPriority(tid, osPriorityHigh - 1);

//     /* refresh timer */
//     timer_id = osTimerNew ((void *) led_row_refresh, osTimerPeriodic, NULL, NULL);
//     osTimerStart (timer_id, 5);
// }


#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "bsp.h"
 #include "bsp2.h"
#include "cmsis_os2.h"
#include "cross_team_definitions.h"
#include "motor.h"


struct GESTURE_COMMAND_PACKET command_array[1000];
enum COMMAND_TYPE GetCommandType(const char buf[],int n);
void PushGestureIntoQueue(struct GESTURE_COMMAND_PACKET _packet);
struct GESTURE_COMMAND_PACKET parseGesturePacket(const char buf[],unsigned int n);
void communication_init(enum DEVICE_MODE _thisDeviceMode);
void DispatchCommand(enum COMMAND_TYPE, void* data );
void DispatchGestureCommand(struct GESTURE_COMMAND_PACKET);
// void DispatchActivateCommand(struct ACTIVATE_COMMAND_PACKET);
// void DispatchDeactivateCommand(struct DEACTIVATE_COMMAND_PACKET);
// void DispatchPathInfoDownloadCommand(struct PATH_INFO_DOWNLOAD);






void my_radio_callback(const char buf[], unsigned int n)
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



/* To be manually changed and compiled for each microbit device
Two microbit cannot have same device MODE*/
enum DEVICE_MODE thisDeviceMode;





// // extern void uart_command_task(void *arg);

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



 void board_init(void)
 {
    leds_init();
    buttons_init();
    uart_init(UART_RX, UART_TX);
 audio_init(SPEAKER, MIC, RUN_MIC);
  radio_init(my_radio_callback);


  communication_init(COMMANDER);
//     return;
 }

int main(void)
{
    /* Initialiazation */
    board_init();
    led_on(0,0);
    /* Greetings */
    printf("hello, world!\n");
    audio_sweep(100, 2000, 200);

   int  led_button_number = 0;
    while(1)
    {
        if(button_get(0) == 1 )
        {
                if(thisDeviceMode == COMMANDER)
                    {
                        struct GESTURE_COMMAND_PACKET gcp;
                        gcp.command = FRONT;
                        gcp.rpm = led_button_number%5;
                        led_button_number = led_button_number + 1;
                        DispatchCommand(GESTURE,(void *)(&gcp));
                    }
                    for (volatile int i=0;i<1000;++i){
                        printf("%i\n",i);   
                    };
        }
    }
    
    //while(1){};


    /* Initialize and start the kernel */
//    osKernelInitialize();
   // osThreadNew(uart_command_task, NULL, NULL);
   // led_blink(1,1);
//    osKernelStart();
    /* never returns */

    led_blink(2, 2);

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

        for (int r = 0; r < LED_NUM_ROWS; r++) {
        for (int c = 0; c < LED_NUM_COLS; c++) {
            led_off(r,c);
        }
    }
    int led_button_number = _packet.rpm;
    led_on(led_button_number,led_button_number);

    // Use rpm for speed (scale as needed)
    int speed = _packet.rpm * 20; // scale 0-5 to 0-100

    // Motor control based on command
    switch (_packet.command) {
        case FRONT:
            motor_on(MOTOR_FORWARD, speed, MOTOR_FORWARD, speed);
            break;
        case BACK:
            motor_on(MOTOR_REVERSE, speed, MOTOR_REVERSE, speed);
            break;
        default:
            // Stop motors for unknown command
            motor_off();
            break;
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
        // case ACTIVATE_BOT:
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



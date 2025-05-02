#include "commander.h"

int current_os_time = 0;
int previousHeartbeatTime = 0;
int saviour_dispatched = 0;
osMessageQueueId_t rescueGestureQueueId;


void commander_init(void)
{
     osThreadNew(CommandSenderThread, NULL, NULL);
     led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);
         if (led_refresh_timer == NULL) {
        printf("LED timer creation failed!\n");
    }
    radio_init(commander_radio_callback);

    /*Initialize the rescue gesture queue*/
    InitRescueGestureQueue();

     

};

void InitRescueGestureQueue(void)
{
    rescueGestureQueueId = osMessageQueueNew(50, sizeof(struct RESCUE_GESTURE_COMMAND_PACKET), NULL);
    if (rescueGestureQueueId == NULL) {
        printf("Rescue gesture queue creation failed!\n");
    }
    else
    {
        printf("Rescue gesture queue created successfully!\n");
    }
}


void CommandSenderThread(void *argument)
{  
    OS_READY = 1;
            osStatus_t status = osTimerStart(led_refresh_timer, 5);
         printf("LED timer start status: %d\n", (int)status);
    enum GESTURE_COMMAND prev_gesture = -1; // Initialize to an invalid value

    while (1)
    {

        check_connection_status();
        
        // frame_buffer[4][4] = 1;  // Turn on
        // 1. Detect current gesture
        enum GESTURE_COMMAND current_gesture = compute_direction();



        // 2. If gesture changed, send new command
        if (current_gesture != prev_gesture && current_gesture != NO_GESTURE)
        {
                     
         osDelay(200); 

            struct GESTURE_COMMAND_PACKET gcp;
            gcp.command = current_gesture;
            gcp.rpm = 3; // Or any speed you want
            DispatchCommand(GESTURE, (void *)&gcp);


            struct RESCUE_GESTURE_COMMAND_PACKET rgcp;
            rgcp.command = current_gesture;
            rgcp.rpm = 3; // Or any speed you want
            rgcp.seconds_elapsed = 5;
            osMessageQueuePut(rescueGestureQueueId, &rgcp, 0, 0);


            
            
            
        }
        prev_gesture = current_gesture;

        // 3. Wait before checking again
       
        osDelay(500); // 200 ms, adjust as needed


        // fflush(stdout); 
        
        // osThreadExit();
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

            previousHeartbeatTime = osKernelGetTickCount()/osKernelGetTickFreq();
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

void check_connection_status(void)
{
    current_os_time = osKernelGetTickCount()/osKernelGetTickFreq();
    seconds_elapsed = current_os_time - previousHeartbeatTime;

    /*Check for heartbeat delay first*/
    if(seconds_elapsed > MAX_HEARTBEAT_DELAY)
    {
        // frame_buffer[4][0] = 1;  // Turn on
        
        if(saviour_dispatched == 0)
        {
            audio_sweep(500,1000,1000);
            saviour_dispatched = 1;
            ActivateSaviour();
            osDelay(1000);
            SendAllCommandsToSaviour();
            osDelay(1000);
            TellSaviourToSearch();
            osDelay(1000);
            osThreadExit();
            /*Send the rescue gesture commands to the saviour*/
 

        }
    }
}

void ActivateSaviour(void)
{
            struct ACTIVATE_COMMAND_PACKET activateCommandPacket;
            activateCommandPacket.device_mode = SAVIOR;
            DispatchActivateCommand(activateCommandPacket);
            saviour_dispatched = 1;
            osDelay(2000);
};

void SendAllCommandsToSaviour(void)
{
    
    // printf("%d",osMessageQueueGetCount(rescueGestureQueueId));
    struct RESCUE_GESTURE_COMMAND_PACKET rgcp;
    frame_buffer[2][2] = 1;
    while(osMessageQueueGet(rescueGestureQueueId, &rgcp, NULL, 0) == osOK)
    {   enum GESTURE_COMMAND command = rgcp.command;
        if(command != NO_GESTURE)
        { 
            // load_letter_to_framebuffer(LETTER_F);
            // frame_buffer[3][2] = 0;
            // printf("Command: %d\n", command);
             DispatchCommand(RESCUE_GESTURE, (void *)&rgcp);
            if(command == FRONT)
            {
                load_letter_to_framebuffer(LETTER_F);
                printf("F\n");
            }
            else if(command == RIGHT)
            {
                load_letter_to_framebuffer(LETTER_R);
                printf("R\n");
            }
            else if(command == BACK)
            {
                load_letter_to_framebuffer(LETTER_B);
                printf("B\n");
            }
            else if(command == LEFT)
            {
                load_letter_to_framebuffer(LETTER_L);
                printf("L\n");
            }

        }
        osDelay(1000);
    }
 
    

};

void TellSaviourToSearch()
{
    struct SEARCH_BEGIN_COMMAND_PACKET searchBeginCommandPacket;
    searchBeginCommandPacket.device_mode = SAVIOR;
    DispatchCommand(SEARCH_BEGIN, (void *)&searchBeginCommandPacket);
};
    

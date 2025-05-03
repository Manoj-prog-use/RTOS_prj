#include "saviour.h"

int SAVIOUR_ACTIVE = 0;
int search_allowed = 0;
int time_savior_search_started = 0;
osMessageQueueId_t saviourRescueGestureQueueId;


void savior_init(void)
{
    motor_off();
    radio_init(savior_radio_callback);
    osThreadId_t savior_thread = osThreadNew(SaviorMainThread, NULL, NULL);
    osThreadId_t savior_search_thread = osThreadNew(SaviorSearchThread, NULL, NULL);
    // osThreadId_t savior_act_on_rescue_gesture_thread = osThreadNew(ActOnRescueGestureThread, NULL, NULL);
    if(savior_thread == NULL){
        led_blink(0,0);
    }
    
    led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);
    InitSaviourRescueGestureQueue();


};


void savior_radio_callback(const char buf[], unsigned int n)
{
     if(GetCommandType(buf,n) == ACTIVATE_BOT && SAVIOUR_ACTIVE == 0)
     {
        struct ACTIVATE_COMMAND_PACKET activateCommandPacket = parseActivatePacket(buf,n);
        if(activateCommandPacket.device_mode == SAVIOR)
        {
            SAVIOUR_ACTIVE = 1;
        }
     }
     else if(GetCommandType(buf,n) == RESCUE_GESTURE && SAVIOUR_ACTIVE == 1)
     {
        struct RESCUE_GESTURE_COMMAND_PACKET rgcp = parseRescueGesturePacket(buf,n);
        osMessageQueuePut(saviourRescueGestureQueueId, &rgcp, 0, 0);
        frame_buffer[4][2] = 1;
     }
     else if(GetCommandType(buf,n) == SEARCH_BEGIN && SAVIOUR_ACTIVE == 1)
     {
        struct SEARCH_BEGIN_COMMAND_PACKET searchBeginCommandPacket = parseSearchBeginPacket(buf,n);
        if(searchBeginCommandPacket.device_mode == SAVIOR)
        {
            time_savior_search_started = osKernelGetTickCount() / osKernelGetTickFreq();
            search_allowed = 1;
            frame_buffer[4][3] = 1;
        }
     }
};

struct ACTIVATE_COMMAND_PACKET parseActivatePacket(const char buf[], unsigned int n)
{
    struct ACTIVATE_COMMAND_PACKET result;
    memcpy(&result, buf + 1, sizeof(struct ACTIVATE_COMMAND_PACKET));
    return result;
};

struct RESCUE_GESTURE_COMMAND_PACKET parseRescueGesturePacket(const char buf[], unsigned int n)
{
    struct RESCUE_GESTURE_COMMAND_PACKET result;
    memcpy(&result, buf + 1, sizeof(struct RESCUE_GESTURE_COMMAND_PACKET));
    return result;
};

struct SEARCH_BEGIN_COMMAND_PACKET parseSearchBeginPacket(const char buf[], unsigned int n)
{
    struct SEARCH_BEGIN_COMMAND_PACKET result;
    memcpy(&result, buf + 1, sizeof(struct SEARCH_BEGIN_COMMAND_PACKET));
    return result;
};

void SaviorMainThread(void *argument)
{
    load_letter_to_framebuffer(LETTER_S);
    osDelay(1000);
    osTimerStart(led_refresh_timer, 5);
    while(1)
    {
        if(SAVIOUR_ACTIVE == 1)
        {
            frame_buffer[4][0] = 1;
           osDelay(1000);
        }
    }
};

void SaviorSearchThread(void *argument)
{
    frame_buffer[2][2] = 1;
    struct RESCUE_GESTURE_COMMAND_PACKET prev_gcp;
    int has_prev = 0;

    while(1)
    {
        if(search_allowed == 1)
        {
            //if queue empty then break out of all loops 
            struct RESCUE_GESTURE_COMMAND_PACKET gcp;
            int processed_gcp = 0;
            while(osMessageQueueGet(saviourRescueGestureQueueId, &gcp, NULL, 0) == osOK)
            {
                processed_gcp = 1;
                if (has_prev) {
                    // Calculate how long to run the previous command
                    int delay = gcp.seconds_elapsed - prev_gcp.seconds_elapsed;
                    if (delay > 0) osDelay(delay * 1000);
                    // After the delay, turn off the motors for the previous command
                    motor_off();
                }

                // Now, start the new command immediately
                switch (gcp.command) {
                    case FRONT:
                        load_letter_to_framebuffer(LETTER_F);
                        motor_on(MOTOR_FORWARD, 50, MOTOR_REVERSE, 50);
                        break;
                    case RIGHT:
                        load_letter_to_framebuffer(LETTER_R);
                        motor_on(MOTOR_FORWARD, 50, MOTOR_FORWARD, 50);
                        break;
                    case BACK:
                        load_letter_to_framebuffer(LETTER_B);
                        motor_on(MOTOR_REVERSE, 50, MOTOR_FORWARD, 50);
                        break;
                    case LEFT:
                        load_letter_to_framebuffer(LETTER_L);
                        motor_on(MOTOR_REVERSE, 50, MOTOR_REVERSE, 50);
                        break;
                    default:
                        motor_off();
                        break;
                }
                prev_gcp = gcp;
                has_prev = 1;
            }
            // After the last command, you may want to turn off the motors after a fixed time or when search ends
            // motor_off();
            if (processed_gcp) {
                osDelay(1000);
                motor_off();
            }
        }
    }
}

void InitSaviourRescueGestureQueue(void)
{
    saviourRescueGestureQueueId = osMessageQueueNew(50, sizeof(struct RESCUE_GESTURE_COMMAND_PACKET), NULL);
    if (saviourRescueGestureQueueId == NULL) {
        printf("Saviour rescue gesture queue creation failed!\n");
    }
}

void ActOnRescueGestureThread(void *argument)
{
    while (1)
    {
        struct RESCUE_GESTURE_COMMAND_PACKET gcp;
        // Wait for a rescue gesture command
        osStatus_t status = osMessageQueueGet(saviourRescueGestureQueueId, &gcp, NULL, osWaitForever);
        if (status == osOK)
        {
            // Calculate how long to move
            int current_time = osKernelGetTickCount() / osKernelGetTickFreq();
            int move_time = gcp.seconds_elapsed - (current_time - time_savior_search_started);

            if (move_time > 0)
            {
                // Act on the gesture
                switch (gcp.command)
                {
                    case FRONT:
                        motor_on(MOTOR_FORWARD, 50, MOTOR_REVERSE, 50);
                        load_letter_to_framebuffer(LETTER_F);
                        break;
                    case BACK:
                        motor_on(MOTOR_REVERSE, 50, MOTOR_FORWARD, 50);
                        load_letter_to_framebuffer(LETTER_B);
                        break;
                    case RIGHT:
                        motor_on(MOTOR_FORWARD, 50, MOTOR_FORWARD, 50);
                        load_letter_to_framebuffer(LETTER_R);
                        break;
                    case LEFT:
                        motor_on(MOTOR_REVERSE, 50, MOTOR_REVERSE, 50);
                        load_letter_to_framebuffer(LETTER_L);
                        break;
                    default:
                        motor_off();
                        break;
                }
                // Move for the calculated time
                osDelay(move_time * 1000); // move_time is in seconds, osDelay expects ms
                motor_off();
            }
            else
            {
                // If move_time <= 0, do nothing or just turn off motors
                motor_off();
            }
        }
    }
}

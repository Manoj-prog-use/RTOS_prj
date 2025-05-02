#include "saviour.h"

int SAVIOUR_ACTIVE = 0;
int search_allowed = 0;
osMessageQueueId_t saviourRescueGestureQueueId;


void savior_init(void)
{
    radio_init(savior_radio_callback);
    osThreadId_t savior_thread = osThreadNew(SaviorMainThread, NULL, NULL);
    osThreadId_t savior_search_thread = osThreadNew(SaviorSearchThread, NULL, NULL);
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
    while(1)
    {
        if(search_allowed == 1)
        {
            
            struct RESCUE_GESTURE_COMMAND_PACKET  gcp;
            while(osMessageQueueGet(saviourRescueGestureQueueId, &gcp, NULL, 0) == osOK)
            {
                if(gcp.command == FRONT)
                {
                    load_letter_to_framebuffer(LETTER_F);
                }
                else if(gcp.command == RIGHT)
                {
                    load_letter_to_framebuffer(LETTER_R);
                }
                else if(gcp.command == BACK)
                {
                    load_letter_to_framebuffer(LETTER_B);
                }
                else if(gcp.command == LEFT)
                {
                    load_letter_to_framebuffer(LETTER_L);
                }
                osDelay(1000);
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

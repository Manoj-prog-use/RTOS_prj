#include "saviour.h"

int SAVIOUR_ACTIVE = 0;

void savior_init(void)
{
    radio_init(savior_radio_callback);
                 osThreadId_t savior_thread = osThreadNew(SaviorMainThread, NULL, NULL);
             if(savior_thread == NULL)
             {
                led_blink(0,0);
             }
    led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);

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

};

struct ACTIVATE_COMMAND_PACKET parseActivatePacket(const char buf[], unsigned int n)
{
    struct ACTIVATE_COMMAND_PACKET result;
    memcpy(&result, buf + 1, sizeof(struct ACTIVATE_COMMAND_PACKET));
    return result;
};

void SaviorMainThread(void *argument)
{
    osTimerStart(led_refresh_timer, 5);
    while(1)
    {
        if(SAVIOUR_ACTIVE == 1)
        {
            frame_buffer[4][0] = 1;
            osDelay(500);
            frame_buffer[4][0] = 0;
            osDelay(500);
        }
    }
};

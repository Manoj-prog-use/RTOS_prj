#include "debug.h"

osTimerId_t test_timer;  // Add this with other global declarations

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

void test_timer_callback(void *argument)
{
    printf("Test timer tick\n");
}
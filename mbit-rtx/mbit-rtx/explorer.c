#include "explorer.h"

osMessageQueueId_t gestureQueueId;
int last_message_time = 0;

void explorer_init(void)
{
    InitializeGestureQueue();
    osThreadNew(GestureQueueThread, NULL, NULL);
    osThreadNew(ExplorerHeartbeatThread, NULL, NULL);
    osThreadNew(ExplorerCheckLostConnection, NULL, NULL);
    led_refresh_timer = osTimerNew((void *)led_row_refresh, osTimerPeriodic, NULL, NULL);
    if (led_refresh_timer == NULL) {
        printf("LED timer creation failed!\n");
    }
    radio_init(explorer_radio_callback);


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
            last_message_time = osKernelGetTickCount() / osKernelGetTickFreq();
        }
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
    load_letter_to_framebuffer(LETTER_E);
    osDelay(1000);
    OS_READY = 1;
    osTimerStart(led_refresh_timer, 5);
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
    int speed = 45;

    switch (gesture) {
        case FRONT:
            // Move forward
            motor_on(MOTOR_FORWARD, speed, MOTOR_REVERSE, speed);
            load_letter_to_framebuffer(LETTER_F);
            break;
        case BACK:
            // Move backward
            motor_on(MOTOR_REVERSE, speed, MOTOR_FORWARD, speed);
            load_letter_to_framebuffer(LETTER_B);
            break;
        case RIGHT:
            // Turn right (left wheel forward, right wheel backward)
            motor_on(MOTOR_FORWARD, 15, MOTOR_FORWARD, 15);
            load_letter_to_framebuffer(LETTER_R);
            break;
        case LEFT:
            // Turn left (left wheel backward, right wheel forward)
            motor_on(MOTOR_REVERSE, 15, MOTOR_REVERSE, 15);
            load_letter_to_framebuffer(LETTER_L);
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

void ExplorerCheckLostConnection(void *argument){

    audio_sweep(500, 1000, 1000);

    while(1){

    int current_time =  osKernelGetTickCount() / osKernelGetTickFreq();

    int delay = current_time - last_message_time;
        
    if(delay>6){
        load_letter_to_framebuffer(LETTER_O);
        printf("O\n");
        motor_off();
    }

    osDelay(1000);
    }

}

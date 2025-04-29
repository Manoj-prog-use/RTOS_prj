#include "explorer.h"

osMessageQueueId_t gestureQueueId;

void explorer_init(void)
{
           InitializeGestureQueue();
           osThreadNew(GestureQueueThread, NULL, NULL);
           osThreadNew(ExplorerHeartbeatThread, NULL, NULL);
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

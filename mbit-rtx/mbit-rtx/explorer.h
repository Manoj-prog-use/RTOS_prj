#ifndef EXPLORER_H
#define EXPLORER_H

#include "cross_team_definitions.h"
#include "cmsis_os2.h"

/*EXPLORER*/
extern osMessageQueueId_t gestureQueueId;
void explorer_init(void);
void explorer_radio_callback(const char buf[], unsigned int n);
void InitializeGestureQueue(void);
void GestureQueueThread(void *argument);
void ExplorerHeartbeatThread(void *argument);
void ActOnGestureCommand(enum GESTURE_COMMAND gesture, int rpm);
void PushGestureIntoQueue(struct GESTURE_COMMAND_PACKET _packet);
struct GESTURE_COMMAND_PACKET parseGesturePacket(const char buf[],unsigned int n);
extern int last_message_time;
/*EXPLORER*/


#endif

#ifndef COMMANDER_H
#define COMMANDER_H

#include "cross_team_definitions.h"
#include "accel.h"

#define MAX_HEARTBEAT_DELAY 10

/*COMMANDER*/
// extern struct RESCUE_GESTURE_COMMAND_PACKET rescue_gesture_command_packet_array[50];
extern osMessageQueueId_t rescueGestureQueueId;
void InitRescueGestureQueue(void);
void commander_init(void);
void commander_radio_callback(const char buf[], unsigned int n);
void CommandSenderThread(void *argument);
struct HEARTBEAT_COMMAND_PACKET parseHeartbeatPacket(const char buf[], unsigned int n);
void PushHeartbeatIntoQueue(struct HEARTBEAT_COMMAND_PACKET _packet);
void check_connection_status(void);
void ActivateSaviour(void);
void SendAllCommandsToSaviour(void);
void TellSaviourToSearch(void);

/*COMMANDER*/
#endif

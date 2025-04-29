#ifndef COMMANDER_H
#define COMMANDER_H

#include "cross_team_definitions.h"

#define MAX_HEARTBEAT_DELAY 5

/*COMMANDER*/
void commander_init(void);
void commander_radio_callback(const char buf[], unsigned int n);
void CommandSenderThread(void *argument);
struct HEARTBEAT_COMMAND_PACKET parseHeartbeatPacket(const char buf[], unsigned int n);
void PushHeartbeatIntoQueue(struct HEARTBEAT_COMMAND_PACKET _packet);
void check_connection_status(void);
/*COMMANDER*/
#endif

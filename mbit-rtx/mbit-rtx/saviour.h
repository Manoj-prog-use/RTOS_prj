#ifndef SAVIOUR_H
#define SAVIOUR_H

#include "cross_team_definitions.h"
#include "cmsis_os2.h"


/*SAVIOR*/
extern int search_allowed;
extern int SAVIOUR_ACTIVE;


void InitSaviourRescueGestureQueue(void);
extern osMessageQueueId_t saviourRescueGestureQueueId;
void savior_init(void);
void savior_radio_callback(const char buf[], unsigned int n);
struct ACTIVATE_COMMAND_PACKET parseActivatePacket(const char buf[], unsigned int n);
struct RESCUE_GESTURE_COMMAND_PACKET parseRescueGesturePacket(const char buf[], unsigned int n);
struct SEARCH_BEGIN_COMMAND_PACKET parseSearchBeginPacket(const char buf[], unsigned int n);
void SaviorMainThread(void *argument);
void SaviorSearchThread(void *argument);
/*SAVIOR*/






#endif

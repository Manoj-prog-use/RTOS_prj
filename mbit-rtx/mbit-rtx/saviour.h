#ifndef SAVIOUR_H
#define SAVIOUR_H

#include "cross_team_definitions.h"
#include "cmsis_os2.h"


/*SAVIOR*/

extern int SAVIOUR_ACTIVE;

void savior_init(void);
void savior_radio_callback(const char buf[], unsigned int n);
struct ACTIVATE_COMMAND_PACKET parseActivatePacket(const char buf[], unsigned int n);
void SaviorMainThread(void *argument);
/*SAVIOR*/






#endif

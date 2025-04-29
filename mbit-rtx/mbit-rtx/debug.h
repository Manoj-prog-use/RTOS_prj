#ifndef DEBUG_H
#define DEBUG_H

#include "cross_team_definitions.h"
#include "cmsis_os2.h"



/*DEBUG*/
void debug_init(void);
void DebugThread(void *argument);
void test_timer_callback(void *argument);
/*DEBUG*/

#endif

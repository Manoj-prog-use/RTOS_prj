#include "cross_team_definitions.h"
#include "explorer.h"
#include "commander.h"
#include "saviour.h"
#include "debug.h"


int main(void)
{

    OS_READY = 0;
    board_init();
    printf("hello, world!\n");
    osKernelInitialize();

    thisDeviceMode = COMMANDER;

    if(thisDeviceMode == EXPLORER)
        {
            explorer_init();
        }
    else if(thisDeviceMode == COMMANDER)
        {
            commander_init();
        }
    else if(thisDeviceMode == SAVIOR)
        {
            savior_init();
        }
    else if(thisDeviceMode == DEBUG || thisDeviceMode == DEVICE_MODE_UNDEFINED)
        {
            debug_init();
        }
    
    osKernelStart();  /* never returns */
    return 0;
}
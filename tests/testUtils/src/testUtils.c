#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "testUtils.h"


static void handler_SIGSEGV(int sig){
    TEST("Check against segmentation exception", 0);
    exit(1);
}


int testUtilsStart(void){
    /* register signal handlers */
    signal(SIGSEGV, handler_SIGSEGV);

    return 0;
}


int testUtilsStop(void){
    /* in future, might add some simple statistics */
    return 0;
}

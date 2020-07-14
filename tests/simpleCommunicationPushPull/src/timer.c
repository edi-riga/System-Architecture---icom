#include <stdlib.h>
#include <time.h>

#include "timer.h"

/* global time struct */
struct timespec tval_start;
struct timespec tval_end;


unsigned timer_ms_start(){
    clock_gettime(CLOCK_MONOTONIC, &tval_start);
    return tval_start.tv_sec*1000 + tval_start.tv_nsec/1000000;
}


unsigned timer_ms_stop(){
    clock_gettime(CLOCK_MONOTONIC, &tval_end);
    return (tval_end.tv_sec  - tval_start.tv_sec)*1000 + 
           (tval_end.tv_nsec - tval_start.tv_nsec)/1000000;
}

unsigned timer_us_start(){
    clock_gettime(CLOCK_MONOTONIC, &tval_start);
    return tval_start.tv_sec*1000000 + tval_start.tv_nsec/1000;
}


unsigned timer_us_stop(){
    clock_gettime(CLOCK_MONOTONIC, &tval_end);
    return (tval_end.tv_sec  - tval_start.tv_sec)*1000000 + 
           (tval_end.tv_nsec - tval_start.tv_nsec)/1000;
}

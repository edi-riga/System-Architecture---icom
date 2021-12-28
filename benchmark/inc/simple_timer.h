#ifndef _SIMPLE_TIMER_H_
#define _SIMPLE_TIMER_H_

#include <stdint.h>

void stimer_set();
uint64_t stimer_get_ns();
uint64_t stimer_get_us();
uint64_t stimer_get_ms();


#endif

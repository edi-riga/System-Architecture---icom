#include <stdint.h>
#include <time.h>
#include "simple_timer.h"

/* private storage for nanoseconds */
static uint64_t set_ns;

void stimer_set(){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  set_ns = now.tv_sec*1e+9 + now.tv_nsec;
}

uint64_t stimer_get_ns(){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (uint64_t)((now.tv_sec*1e+9 + now.tv_nsec)-set_ns);
}

uint64_t stimer_get_us(){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (uint64_t)(1e-3*((now.tv_sec*1e+9 + now.tv_nsec)-set_ns));
}

uint64_t stimer_get_ms(){
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (uint64_t)(1e-6*((now.tv_sec*1e+9 + now.tv_nsec)-set_ns));
}

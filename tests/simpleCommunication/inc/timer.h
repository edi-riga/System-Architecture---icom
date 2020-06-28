#ifndef _TIMER_H_
#define _TIMER_H_

/* Start time-measurement procedure 
 *
 * @return Timestamp in miliseconds
 **/
unsigned timer_ms_start();

/* Stop time-measurement procedure 
 *
 * @return Time difference in miliseconds
 **/
unsigned timer_ms_stop();

/* Start time-measurement procedure 
 *
 * @return Timestamp in microseconds
 **/
unsigned timer_us_start();

/* Stop time-measurement procedure 
 *
 * @return Time difference in microseconds
 **/
unsigned timer_us_stop();

#endif

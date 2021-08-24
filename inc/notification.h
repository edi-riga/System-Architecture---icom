#ifndef _NOTIFICATION_H_
#define _NOTIFICATION_H_

#include <stdio.h>
#include <string.h>

/* COLORS */
#define COLOR_DEFAULT  "\033[0m"
#define COLOR_RED      "\033[1;31m"
#define COLOR_GREEN    "\033[0;32m"
#define COLOR_YELLOW   "\033[1;33m"
#define COLOR_GRAY     "\033[0;97m"


/* INFO */
#define _I(fmt,args...)\
printf(fmt "\n", ##args); fflush(stdout)

/* ERROR */
#define _E(fmt,args...)\
printf(COLOR_RED "ERROR: " fmt "\n" COLOR_DEFAULT, ##args)

/* WARNING */
#define _W(fmt,args...)\
printf(COLOR_YELLOW "WARNING: " fmt "\n" COLOR_DEFAULT, ##args)

/* SYSTEM ERROR */
#define _SE(fmt,args...)\
printf(COLOR_RED "SYSTEM ERROR (%s): " fmt "\n" COLOR_DEFAULT, strerror(errno), ##args)

/* SYSTEM WARNING  */
#define _SW(fmt,args...)\
printf(COLOR_YELLOW "SYSTEM WARNING (%s): " fmt "\n" COLOR_DEFAULT, strerror(errno), ##args)

/* DEBUGGING */
#ifdef DEBUG
    #define _D(fmt,args...)\
    printf(COLOR_GRAY "DEBUG:%s:%u: "fmt "\n" COLOR_DEFAULT, __func__, __LINE__, ##args); fflush(stdout)
#else
    #define _D(fmt,args...)    
#endif


#endif

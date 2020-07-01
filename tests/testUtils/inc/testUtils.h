#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include<stdio.h>

/* Colors */
#define COLOR_DEFAULT  "\033[0m"
#define COLOR_RED      "\033[1;31m"
#define COLOR_GREEN    "\033[0;32m"

/* Test routine */
#define TEST(testName, assert)                                                 \
{                                                                              \
   if(assert){                                                                \
      printf("TEST: %-60s %s%s%s\n", testName, COLOR_GREEN, "PASSED", COLOR_DEFAULT);\
   } else {                                                                   \
      printf("TEST: %-60s %s%s%s\n", testName, COLOR_RED,   "FAILED", COLOR_DEFAULT);\
   }                                                                          \
}

/* Testing API (handlers, statistics, etc.) */
int testUtilsStart(void);
int testUtilsStop(void);

#endif

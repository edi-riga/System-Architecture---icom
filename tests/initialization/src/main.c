#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "icom.h"

#define _I(fmt, args...)   printf(fmt "\n", ##args)
#define COLOR_DEFAULT  "\033[0m"
#define COLOR_RED      "\033[1;31m"
#define COLOR_GREEN    "\033[0;32m"

#define TEST(testName, assert)                                                 \
{                                                                              \
   if(assert){                                                                \
      printf("TEST: %-60s %s%s%s\n", testName, COLOR_GREEN, "PASSED", COLOR_DEFAULT);\
   } else {                                                                   \
      printf("TEST: %-60s %s%s%s\n", testName, COLOR_RED,   "FAILED", COLOR_DEFAULT);\
   }                                                                          \
}

static void handler_sigsegv(int sig){
    TEST("Check against segmentation exception", 0);
    exit(1);
}


int main(void){
    int ret = 0;
    icom_t *icomPush, *icomPull;

    signal(SIGSEGV, handler_sigsegv);

    ret = icom_init();
    TEST("API initialization", ret == 0);

    
    icomPush = icom_initPush("inproc://tmp", 1024, 2, ICOM_DEFAULT);
    TEST("PUSH socket initialization", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH socket deinitialization", 1);

    icomPush = icom_initPush("inproc://tmp", 1024, 2, ICOM_ZERO_COPY);
    TEST("PUSH socket initialization (ZERO COPY)", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH socket deinitialization (ZERO COPY)", 1);

    icomPush = icom_initPush("inproc://tmp", 1024, 2, ICOM_ZERO_COPY | ICOM_PROTECTED);
    TEST("PUSH socket initialization (ZERO COPY, PROTECTED)", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH socket deinitialization (ZERO COPY, PROTECTED)", 1);


    icomPush = icom_initPush("tcp://127.0.0.1:8888", 1024, 2, ICOM_DEFAULT);
    TEST("PUSH IP socket initialization", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH IP socket deinitialization", 1);

    icomPush = icom_initPush("tcp://127.0.0.1:8888", 1024, 2, ICOM_ZERO_COPY);
    TEST("PUSH socket initialization (ZERO COPY)", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH socket deinitialization (ZERO COPY)", 1);

    icomPush = icom_initPush("tcp://127.0.0.1:8888", 1024, 2, ICOM_ZERO_COPY | ICOM_PROTECTED);
    TEST("PUSH socket initialization (ZERO COPY, PROTECTED)", icomPush != NULL);

    icom_deinit(icomPush);
    TEST("PUSH socket deinitialization (ZERO COPY, PROTECTED)", 1);


    icomPull = icom_initPull("inproc://tmp", 1024, ICOM_DEFAULT);
    TEST("PULL socket initialization", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL socket deinitialization", 1);

    icomPull = icom_initPull("inproc://tmp", 1024, ICOM_ZERO_COPY);
    TEST("PULL socket initialization (ZERO COPY)", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL socket deinitialization (ZERO COPY)", 1);

    icomPull = icom_initPull("inproc://tmp", 1024, ICOM_ZERO_COPY | ICOM_PROTECTED);
    TEST("PULL socket initialization (ZERO COPY, PROTECTED)", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL socket deinitialization (ZERO COPY, PROTECTED)", 1);


    icomPull = icom_initPull("tcp://127.0.0.1:8888", 1024, ICOM_DEFAULT);
    TEST("PULL IP socket initialization", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL IP socket deinitialization", 1);

    icomPull = icom_initPull("tcp://127.0.0.1:8888", 1024, ICOM_ZERO_COPY);
    TEST("PULL socket initialization (ZERO COPY)", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL socket deinitialization (ZERO COPY)", 1);

    icomPull = icom_initPull("tcp://127.0.0.1:8888", 1024, ICOM_ZERO_COPY | ICOM_PROTECTED);
    TEST("PULL socket initialization (ZERO COPY, PROTECTED)", icomPull != NULL);

    icom_deinit(icomPull);
    TEST("PULL socket deinitialization (ZERO COPY, PROTECTED)", 1);


    icom_release();
    TEST("API deinitialization", 1);

    return 0;
}

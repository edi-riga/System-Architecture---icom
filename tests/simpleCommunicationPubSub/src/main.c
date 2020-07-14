#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "icom.h"
#include "timer.h"
#include "testUtils.h"

#define _I(fmt, args...)   printf(fmt "\n", ##args); fflush(stdout)
#define PAYLOAD_SIZE    1024
#define TRANSFER_COUNT  8

/* test messages */
const char *TEST_MSG_DEFAULT   = "PUB-SUB communication";
const char *TEST_MSG_ZERO_COPY = "PUB-SUB communication (ZERO COPY)";
const char *TEST_MSG_PROTECTED = "PUB-SUB communication (ZERO COPY, PROTECTED)";

/* globals for tests */
int32_t flags = ICOM_DEFAULT;
const char *testMsg;

void *thread_pub(void *arg){
    icom_t *icom = icom_initPublish((char*)arg, PAYLOAD_SIZE, 2, flags);
    int *buffer;

    for(int i=0; i<TRANSFER_COUNT; i++){
        ICOM_GET_BUFFER(icom, buffer);

        buffer[0] = i;

        ICOM_DO(icom);
    }

    usleep(100); //TODO: Missing packet if deinit before data is sent
    icom_deinit(icom);
    return NULL;
}

void *thread_sub(void *arg){
    icom_t *icom = icom_initSubscribe((char*)arg, PAYLOAD_SIZE, flags);
    int *buffer;

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        ICOM_DO_AND_FOR_EACH_BUFFER(icom, buffer);

        _I("Received buffer: %d (expected: %d)", *buffer, i);
        TEST(testMsg, *buffer == i);

        ICOM_FOR_EACH_END;
    }

    icom_deinit(icom);
    return NULL;
}


int main(void){
    pthread_t pidTx, pidRx;
    icom_t *icomPush, *icomPull;
    unsigned timeDeep, timeZero, timeZeroProtected;

    testUtilsStart();

    _I("Initializing icom API");
    icom_init();

    _I("Initializing pthreads (deep data copy experiment)");
    testMsg = TEST_MSG_DEFAULT;
    flags   = ICOM_DEFAULT;
    timer_us_start();
    pthread_create(&pidTx, NULL, thread_pub, "inproc://tmp0");
    pthread_create(&pidRx, NULL, thread_sub, "inproc://tmp0");

    _I("Waiting for threads to finish");
    pthread_join(pidTx, NULL);
    pthread_join(pidRx, NULL);
    timeDeep = timer_us_stop();


    //_I("Initializing pthreads (zero data copy experiment)");
    //testMsg = TEST_MSG_ZERO_COPY;
    //flags   = ICOM_ZERO_COPY;
    //timer_us_start();
    //pthread_create(&pidTx, NULL, thread_pub, "inproc://tmp1");
    //pthread_create(&pidRx, NULL, thread_sub, "inproc://tmp1");

    //_I("Waiting for threads to finish");
    //pthread_join(pidTx, NULL);
    //pthread_join(pidRx, NULL);
    //timeZero = timer_us_stop();


    //_I("Initializing pthreads (zero data protected copy experiment)");
    //testMsg = TEST_MSG_PROTECTED;
    //flags   = ICOM_PROTECTED;
    //timer_us_start();
    //pthread_create(&pidTx, NULL, thread_pub, "inproc://tmp2");
    //pthread_create(&pidRx, NULL, thread_sub, "inproc://tmp2");

    //_I("Waiting for threads to finish");
    //pthread_join(pidTx, NULL);
    //pthread_join(pidRx, NULL);
    //timeZeroProtected = timer_us_stop();

    _I("TIME (with deep copy):           %u us", timeDeep);
    _I("TIME (with zero copy):           %u us", timeZero);
    _I("TIME (with zero copy, prtected): %u us", timeZeroProtected);

    _I("Deinitializing icom API");
    icom_release();

    return testUtilsStop();
}

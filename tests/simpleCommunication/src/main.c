#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "icom.h"
#include "timer.h"

#define _I(fmt, args...)   printf(fmt "\n", ##args); fflush(stdout)
#define PAYLOAD_SIZE    1024
#define TRANSFER_COUNT  8*1024


void *thread_push(void *arg){
    icom_t *icom = icom_initPush((char*)arg, PAYLOAD_SIZE, 2, 0);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        ((int*)(packet->payload))[0] = i;
        packet = icom_do(icom);
    }

    icom_deinit(icom);
    return NULL;
}

void *thread_pull(void *arg){
    icom_t *icom = icom_initPull((char*)arg, PAYLOAD_SIZE, 0);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        packet = icom_do(icom);
        do{
            packet = packet->next;
        } while(packet != NULL);
    }

    icom_deinit(icom);
    return NULL;
}


void *thread_pushZero(void *arg){
    icom_t *icom = icom_initPush((char*)arg, PAYLOAD_SIZE, 2, ICOM_ZERO_COPY);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        ((int*)(packet->payload))[0] = i;
        packet = icom_do(icom);
    }

    icom_deinit(icom);
    return NULL;
}

void *thread_pullZero(void *arg){
    icom_t *icom = icom_initPull((char*)arg, PAYLOAD_SIZE, ICOM_ZERO_COPY);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        packet = icom_do(icom);
        do{
            packet = packet->next;
        } while(packet != NULL);
    }

    icom_deinit(icom);
    return NULL;
}


void *thread_pushZeroProtected(void *arg){
    icom_t *icom = icom_initPush((char*)arg, PAYLOAD_SIZE, 2, ICOM_ZERO_COPY | ICOM_PROTECTED);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        ((int*)(packet->payload))[0] = i;
        packet = icom_do(icom);
    }

    icom_deinit(icom);
    return NULL;
}

void *thread_pullZeroProtected(void *arg){
    icom_t *icom = icom_initPull((char*)arg, PAYLOAD_SIZE, ICOM_ZERO_COPY | ICOM_PROTECTED);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<TRANSFER_COUNT; i++){
        packet = icom_do(icom);
        do{
            packet = packet->next;
        } while(packet != NULL);
    }

    icom_deinit(icom);
    return NULL;
}


int main(void){
    pthread_t pidTx, pidRx;
    icom_t *icomPush, *icomPull;
    unsigned timeDeep, timeZero, timeZeroProtected;

    _I("Initializing icom API");
    icom_init();

    for(int i=0; i<100; i++){
    
        _I("Initializing pthreads (deep data copy experiment)");
        timer_us_start();
        pthread_create(&pidTx, NULL, thread_push, "inproc://tmp");
        pthread_create(&pidRx, NULL, thread_pull, "inproc://tmp");

        _I("Waiting for threads to finish");
        pthread_join(pidTx, NULL);
        pthread_join(pidRx, NULL);
        timeDeep = timer_us_stop();


        _I("Initializing pthreads (zero data copy experiment)");
        timer_us_start();
        pthread_create(&pidTx, NULL, thread_pushZero, "inproc://tmp");
        pthread_create(&pidRx, NULL, thread_pullZero, "inproc://tmp");

        _I("Waiting for threads to finish");
        pthread_join(pidTx, NULL);
        pthread_join(pidRx, NULL);
        timeZero = timer_us_stop();


        _I("Initializing pthreads (zero data protected copy experiment)");
        timer_us_start();
        pthread_create(&pidTx, NULL, thread_pushZeroProtected, "inproc://tmp");
        pthread_create(&pidRx, NULL, thread_pullZeroProtected, "inproc://tmp");

        _I("Waiting for threads to finish");
        pthread_join(pidTx, NULL);
        pthread_join(pidRx, NULL);
        timeZeroProtected = timer_us_stop();


        _I("TIME (with deep copy):           %u us", timeDeep);
        _I("TIME (with zero copy):           %u us", timeZero);
        _I("TIME (with zero copy, prtected): %u us", timeZeroProtected);
    }

    _I("Deinitializing icom API");
    icom_release();

    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "icom.h"

#define _I(fmt, args...)   printf(fmt "\n", ##args)
#define PAYLOAD_SIZE 1024*1024


void *thread_push(void *arg){
    icom_t *icom = icom_initPush((char*)arg, PAYLOAD_SIZE, 2);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<1024; i++){
        printf("Sending(%s) %d\n", (char*)arg, i);
        ((int*)(packet->payload))[0] = i;
        packet = icom_do(icom);
    }

    icom_deinitPush(icom);
    return NULL;
}


void *thread_pull(void *arg){
    icom_t *icom = icom_initPull((char*)arg, PAYLOAD_SIZE);

    icomPacket_t *packet = icom_getCurrentPacket(icom);
    for(int i=0; i<1024; i++){
        packet = icom_do(icom);
        do{
            printf("Received(%s): %u\n", (char*)arg, ((int*)packet->payload)[0]);
            packet = packet->next;
        } while(packet != NULL);
    }

    icom_deinitPush(icom);
    return NULL;
}


int main(void){
    pthread_t pidTx, pidRx;
    icom_t *icomPush, *icomPull;

    _I("Initializing icom API");
    icom_init();

    _I("Initializing pthreads (deep data copy experiment)");
    pthread_create(&pidTx, NULL, thread_push, "inproc://tmp");
    pthread_create(&pidRx, NULL, thread_pull, "inproc://tmp");

    _I("Waiting for threads to finish");
    pthread_join(pidTx, NULL);
    pthread_join(pidRx, NULL);

    _I("Deinitializing icom API");
    icom_release();

    return 0;
}

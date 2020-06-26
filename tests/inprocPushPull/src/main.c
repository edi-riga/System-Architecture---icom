#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "icom.h"

/* TEST 0 */
char *comTx0[] = {"inproc://a0"};
char *comRx0[] = {"inproc://a0"};

/* TEST 1 */
char *comTx1[] = {"inproc://b[0-4]"};
char *comRx1[] = {
    "inproc://b0",
    "inproc://b1",
    "inproc://b2",
    "inproc://b3",
    "inproc://b4",
};

/* TEST 2 */
char *comTx2[] = {
    "inproc://c0",
    "inproc://c1",
    "inproc://c2",
    "inproc://c3",
    "inproc://c4",
};
char *comRx2[] = {"inproc://c[0-4]"};

/* TEST 3 */
char *comTx3[] = {
    "inproc://d[0-1]",
    "inproc://d[2-3]"
};
char *comRx3[] = {
    "inproc://d0",
    "inproc://d[1-2]",
    "inproc://d3",
};

void *thread_push(void *arg){
    icom_t *icom = icom_initPush((char*)arg, sizeof(int), 2, 0);
    icomBuffer_t *buff = icom_getCurrentBuffer(icom);

    for(int i=0; i<5; i++){
        printf("Sending(%s) %d\n", (char*)arg, i);
        ((int*)(buff->mem))[0] = i;
        buff = icom_do(icom);
    }
    //sleep(1);

    icom_pushDeinit(icom);
    return NULL;
}

void *thread_pull(void *arg){
    icom_t *icom = icom_initPull((char*)arg, sizeof(int), 0);
    icomBuffer_t *buff;

    for(int i=0; i<5; i++){
        //sleep(1);
        buff = icom_do(icom);
        do{
            printf("Received(%s): %u\n", (char*)arg, ((int*)buff->mem)[0]);
            buff = buff->next;
        } while(buff != NULL);
    }

    icom_pushDeinit(icom);
    return NULL;
}

void experiment(char **comTx, char **comRx, int countTx, int countRx){
    pthread_t *pidTx, *pidRx;

    pidTx = (pthread_t*)malloc(countTx*sizeof(*pidTx));
    pidRx = (pthread_t*)malloc(countRx*sizeof(*pidRx));

    /* generate threads */
    for(int i=0; i<countTx; i++)
        pthread_create(&pidTx[i], NULL, thread_push, comTx[i]);
    for(int i=0; i<countRx; i++)
        pthread_create(&pidRx[i], NULL, thread_pull, comRx[i]);

    /* join the threads */
    for(int i=0; i<countTx; i++)
        pthread_join(pidTx[i], NULL);
    for(int i=0; i<countRx; i++)
        pthread_join(pidRx[i], NULL);

    free(pidTx);
    free(pidRx);
}

#define TEST0_PUSH_COUNT (sizeof(comTx0)/sizeof(*comTx0))
#define TEST0_PULL_COUNT (sizeof(comRx0)/sizeof(*comRx0))
#define TEST1_PUSH_COUNT (sizeof(comTx1)/sizeof(*comTx1))
#define TEST1_PULL_COUNT (sizeof(comRx1)/sizeof(*comRx1))
#define TEST2_PUSH_COUNT (sizeof(comTx2)/sizeof(*comTx2))
#define TEST2_PULL_COUNT (sizeof(comRx2)/sizeof(*comRx2))
#define TEST3_PUSH_COUNT (sizeof(comTx3)/sizeof(*comTx3))
#define TEST3_PULL_COUNT (sizeof(comRx3)/sizeof(*comRx3))
int main(void){
    pthread_t pidTx0[TEST0_PUSH_COUNT], pidRx0[TEST0_PULL_COUNT];
    pthread_t pidTx1[TEST1_PUSH_COUNT], pidRx1[TEST1_PULL_COUNT];
    pthread_t pidTx2[TEST2_PUSH_COUNT], pidRx2[TEST2_PULL_COUNT];
    pthread_t pidTx3[TEST3_PUSH_COUNT], pidRx3[TEST3_PULL_COUNT];

    printf("===== TEST 0 (Single Push -> Single Pull) =====\n");
    experiment(comTx0, comRx0, TEST0_PUSH_COUNT, TEST0_PULL_COUNT);

    printf("===== TEST 1 (Single Push -> Multiple Pull) =====\n");
    experiment(comTx1, comRx1, TEST1_PUSH_COUNT, TEST1_PULL_COUNT);

    printf("===== TEST 2 (Multiple Push -> Single Pull) =====\n");
    experiment(comTx2, comRx2, TEST2_PUSH_COUNT, TEST2_PULL_COUNT);

    printf("===== TEST 3 (Multiple Push -> Multiple Pull) =====\n");
    experiment(comTx3, comRx3, TEST3_PUSH_COUNT, TEST3_PULL_COUNT);

    icom_release();
    return 0;
}

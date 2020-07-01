#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "icom.h"
#include "testUtils.h"

const char *MSG_SINGLE2SINGLE     = "Single to single communication";
const char *MSG_SINGLE2MULTIPLE   = "Single to multiple communication";
const char *MSG_MULTIPLE2SINGLE   = "Multiple to single communication";
const char *MSG_MULTIPLE2MULTIPLE = "Multiple to multiple communication";
const char *MSG_SINGLE2SINGLE_ZERO     = "Single to single communication (ZERO COPY)";
const char *MSG_SINGLE2MULTIPLE_ZERO   = "Single to multiple communication (ZERO COPY)";
const char *MSG_MULTIPLE2SINGLE_ZERO   = "Multiple to single communication (ZERO COPY)";
const char *MSG_MULTIPLE2MULTIPLE_ZERO = "Multiple to multiple communication (ZERO COPY)";
const char *MSG_SINGLE2SINGLE_ZERO_PROTECTED     = "Single to single communication (ZERO COPY, PROTECTED)";
const char *MSG_SINGLE2MULTIPLE_ZERO_PROTECTED   = "Single to multiple communication (ZERO COPY, PROTECTED)";
const char *MSG_MULTIPLE2SINGLE_ZERO_PROTECTED   = "Multiple to single communication (ZERO COPY, PROTECTED)";
const char *MSG_MULTIPLE2MULTIPLE_ZERO_PROTECTED = "Multiple to multiple communication (ZERO COPY, PROTECTED)";


/* Global flags */
uint32_t flags = ICOM_DEFAULT;
const char *msgTest;


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
    icom_t *icom = icom_initPush((char*)arg, sizeof(int), 2, flags);
    int *buffer;

    for(int i=0; i<5; i++){
        ICOM_GET_BUFFER(icom, buffer);

        buffer[0] = i;

        ICOM_DO(icom);
    }

    usleep(1000);
    icom_deinit(icom);
    return NULL;
}

void *thread_pull(void *arg){
    icom_t *icom = icom_initPull((char*)arg, sizeof(int), flags);
    int *buffer;

    for(int i=0; i<5; i++){
        ICOM_DO_AND_FOR_EACH_BUFFER(icom, buffer);

        TEST(msgTest, buffer[0] == i);

        ICOM_FOR_EACH_END;
    }

    icom_deinit(icom);
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

    testUtilsStart();

    printf("===== TEST 0 (Single Push -> Single Pull) =====\n");
    msgTest = MSG_SINGLE2SINGLE;
    experiment(comTx0, comRx0, TEST0_PUSH_COUNT, TEST0_PULL_COUNT);

    printf("===== TEST 1 (Single Push -> Multiple Pull) =====\n");
    msgTest = MSG_SINGLE2MULTIPLE;
    experiment(comTx1, comRx1, TEST1_PUSH_COUNT, TEST1_PULL_COUNT);

    printf("===== TEST 2 (Multiple Push -> Single Pull) =====\n");
    msgTest = MSG_MULTIPLE2SINGLE;
    experiment(comTx2, comRx2, TEST2_PUSH_COUNT, TEST2_PULL_COUNT);

    printf("===== TEST 3 (Multiple Push -> Multiple Pull) =====\n");
    msgTest = MSG_MULTIPLE2MULTIPLE;
    experiment(comTx3, comRx3, TEST3_PUSH_COUNT, TEST3_PULL_COUNT);


    printf("Updating flags...");
    flags = ICOM_ZERO_COPY;

    printf("===== TEST 4 (Single Push -> Single Pull) =====\n");
    msgTest = MSG_SINGLE2SINGLE_ZERO;
    experiment(comTx0, comRx0, TEST0_PUSH_COUNT, TEST0_PULL_COUNT);

    printf("===== TEST 5 (Single Push -> Multiple Pull) =====\n");
    msgTest = MSG_SINGLE2MULTIPLE_ZERO;
    experiment(comTx1, comRx1, TEST1_PUSH_COUNT, TEST1_PULL_COUNT);

    printf("===== TEST 6 (Multiple Push -> Single Pull) =====\n");
    msgTest = MSG_MULTIPLE2SINGLE_ZERO;
    experiment(comTx2, comRx2, TEST2_PUSH_COUNT, TEST2_PULL_COUNT);

    printf("===== TEST 7 (Multiple Push -> Multiple Pull) =====\n");
    msgTest = MSG_MULTIPLE2MULTIPLE_ZERO;
    experiment(comTx3, comRx3, TEST3_PUSH_COUNT, TEST3_PULL_COUNT);


    printf("Updating flags...");
    flags = ICOM_ZERO_COPY | ICOM_PROTECTED;

    printf("===== TEST 8 (Single Push -> Single Pull) =====\n");
    msgTest = MSG_SINGLE2SINGLE_ZERO_PROTECTED;
    experiment(comTx0, comRx0, TEST0_PUSH_COUNT, TEST0_PULL_COUNT);

    printf("===== TEST 9 (Single Push -> Multiple Pull) =====\n");
    msgTest = MSG_SINGLE2MULTIPLE_ZERO_PROTECTED;
    experiment(comTx1, comRx1, TEST1_PUSH_COUNT, TEST1_PULL_COUNT);

    printf("===== TEST 10 (Multiple Push -> Single Pull) =====\n");
    msgTest = MSG_MULTIPLE2SINGLE_ZERO_PROTECTED;
    experiment(comTx2, comRx2, TEST2_PUSH_COUNT, TEST2_PULL_COUNT);

    printf("===== TEST 11 (Multiple Push -> Multiple Pull) =====\n");
    msgTest = MSG_MULTIPLE2MULTIPLE_ZERO_PROTECTED;
    experiment(comTx3, comRx3, TEST3_PUSH_COUNT, TEST3_PULL_COUNT);


    icom_release();

    return testUtilsStop();
}

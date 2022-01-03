#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "link_common.h"

extern "C" {
  #include "icom.h"
}

////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////
typedef struct {
  icom_t    *icom;
  void      *buf;
  unsigned   bufSize;
} thread_send_t;

static void* thread_send(void *p){
  thread_send_t *pdata = (thread_send_t*)p;
  return (void*)icom_send(pdata->icom, pdata->buf, pdata->bufSize);
}

////////////////////////////////////////////////////////////////////////////////
// GENERIC TESTS - INITIALIZATION/DEINITIALIZATION
////////////////////////////////////////////////////////////////////////////////
void link_common_initialization(const char *icomStr, unsigned testCount){
  icom_t *icom;
  for(unsigned i=0; i<testCount; i++){
    icom = icom_init(icomStr);
    EXPECT_FALSE(ICOM_IS_ERR(icom));
    icom_deinit(icom);
  }
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - SIMPLE TRANSFER
////////////////////////////////////////////////////////////////////////////////
void link_common_simple(const char *icomTxStr, const char *icomRxStr, uint32_t txBufSize){
  icom_t *icom_rx, *icom_tx;
  thread_send_t thread_pdata;
  pthread_t pid;
  icomStatus_t status;
  uint8_t *rxBuf, *txBuf;
  uint32_t rxBufSize;
  void *ret;

  /* initialize communication objects */
  icom_tx = icom_init(icomTxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_tx));
  icom_rx = icom_init(icomRxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_rx));

  /* initialize random data */
  txBuf = (uint8_t*)malloc(txBufSize);
  for(int i=0; i<txBufSize; i++){
    txBuf[i] = rand() % txBufSize;
  }

  /* initiate send/receive processes */
  thread_pdata = {icom_tx, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icom_rx, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status,    ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_EQ(memcmp(rxBuf, txBuf, txBufSize), 0);

  /* cleanup */
  free(txBuf);
  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);
  icom_deinit(icom_tx);
  icom_deinit(icom_rx);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - VARIED TRANSFERS
////////////////////////////////////////////////////////////////////////////////
void link_common_varied(const char *icomTxStr, const char *icomRxStr, unsigned testCount){
  icom_t *icom_rx, *icom_tx;
  thread_send_t thread_pdata;
  pthread_t pid;
  icomStatus_t status;
  uint8_t *rxBuf, *txBuf = NULL;
  uint32_t rxBufSize, txBufSize;
  void *ret;

  /* initialize communication objects */
  icom_tx = icom_init(icomTxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_tx));
  icom_rx = icom_init(icomRxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_rx));

  for(int i=0; i<testCount; i++){
    txBufSize = rand() % 256;

    /* initialize random data */
    txBuf = (uint8_t*)realloc(txBuf, txBufSize);
    for(int j=0; j<txBufSize; j++){
      txBuf[j] = rand() % txBufSize;
    }

    /* initiate send/receive processes */
    thread_pdata = {icom_tx, txBuf, txBufSize};
    pthread_create(&pid, NULL, thread_send, &thread_pdata);
    status = icom_recv(icom_rx, (void**)&rxBuf, &rxBufSize);
    pthread_join(pid, &ret);

    /* check results */
    EXPECT_EQ(status,    ICOM_SUCCESS);
    EXPECT_EQ(rxBufSize, txBufSize);
    EXPECT_EQ(memcmp(rxBuf, txBuf, txBufSize), 0);
  }

  /* cleanup */
  free(txBuf);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);
  icom_deinit(icom_tx);
  icom_deinit(icom_rx);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - FANIN TRANSFERS
////////////////////////////////////////////////////////////////////////////////
#define TX_COUNT 16
void link_common_fanin(const char *connectStrings[], const char *bindString, unsigned connectCount){
  icom_t         *icomConnect[connectCount], *icomBind;
  thread_send_t   thread_pdata[connectCount];
  pthread_t       pid[connectCount];
  uint8_t         txBuf[connectCount][TX_COUNT], *rxBuf;
  icomStatus_t    status;
  uint32_t        rxBufSize;
  void           *ret;

  /* initialize connect (sender) components */
  for(int i=0; i<connectCount; i++){
    icomConnect[i] = icom_init(connectStrings[i]);
    EXPECT_FALSE(ICOM_IS_ERR(icomConnect[i]));
  }

  /* initialize bind (receiver) components */
  icomBind = icom_init(bindString);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  /* initialize connect (sender) data */
  for(int i=0; i<connectCount; i++){
  for(int j=0; j<TX_COUNT; j++){
    txBuf[i][j] = rand() % 10 + i;
  }}

  /* initialize sender threads */
  for(int i=0; i<connectCount; i++){
    thread_pdata[i] = {icomConnect[i], txBuf[i], TX_COUNT};
    pthread_create(&pid[i], NULL, thread_send, &thread_pdata[i]);
  }

  /* receive all buffers */
  status = icom_recv(icomBind);
  EXPECT_EQ(status, ICOM_SUCCESS);

  /* request buffers one-by-one */
  rxBuf = NULL;
  for(int i=0; i<connectCount; i++){
    icom_nextBuffer(icomBind, (void**)&rxBuf, &rxBufSize);
    EXPECT_EQ(rxBufSize, TX_COUNT);
    EXPECT_EQ(memcmp(rxBuf, txBuf[i], TX_COUNT), 0);
  }

  /* join sender threads */
  for(int i=0; i<connectCount; i++){
    pthread_join(pid[i], &ret);
    EXPECT_TRUE((uint64_t)ret == ICOM_SUCCESS);
  }

  /* deinitialize connect (sender) objects */
  for(int i=0; i<connectCount; i++){
    icom_deinit(icomConnect[i]);
  }

  /* deinitialize bind (receiver) object */
  icom_deinit(icomBind);
}

////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - COMPLEX
////////////////////////////////////////////////////////////////////////////////
void link_common_complex(
const char *connectStrings[], // must have 3 strings
const char *bindStrings[]     // must have 2 strings
)
{
  icom_t *icomBind[2], *icomConnect[3];
  thread_send_t thread_pdata[3];
  const char *txBuf0 = "buffer0";
  const char *txBuf1 = "buffer1";
  const char *txBuf2 = "buffer2";
  uint32_t txBufSize[] = {sizeof(txBuf0), sizeof(txBuf1), sizeof(txBuf2)};
  char *rxBuf; uint32_t rxBufSize;
  icomStatus_t status;
  pthread_t pid[3];
  void *ret;

  icomConnect[0] = icom_init(connectStrings[0]);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect[0]));
  icomConnect[1] = icom_init(connectStrings[1]);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect[1]));
  icomConnect[2] = icom_init(connectStrings[2]);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect[2]));
  icomBind[0] = icom_init(bindStrings[0]);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind[0]));
  icomBind[1] = icom_init(bindStrings[1]);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind[1]));

  thread_pdata[0] = {icomConnect[0], (char*)txBuf0, txBufSize[0]};
  pthread_create(&pid[0], NULL, thread_send, &thread_pdata[0]);
  thread_pdata[1] = {icomConnect[1], (char*)txBuf1, txBufSize[1]};
  pthread_create(&pid[1], NULL, thread_send, &thread_pdata[1]);
  thread_pdata[2] = {icomConnect[2], (char*)txBuf2, txBufSize[2]};
  pthread_create(&pid[2], NULL, thread_send, &thread_pdata[2]);

  status = icom_recv(icomBind[0], (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize[0]);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf0, txBufSize[0]) == 0)
            || (memcmp(rxBuf, txBuf1, txBufSize[1]) == 0));

  icom_nextBuffer(icomBind[0], (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize[1]);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf0, txBufSize[0]) == 0)
            || (memcmp(rxBuf, txBuf1, txBufSize[1]) == 0));

  status = icom_recv(icomBind[1], (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize[1]);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf1, txBufSize[1]) == 0)
            || (memcmp(rxBuf, txBuf2, txBufSize[2]) == 0));

  icom_nextBuffer(icomBind[1], (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize[2]);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf1, txBufSize[1]) == 0)
            || (memcmp(rxBuf, txBuf2, txBufSize[2]) == 0));

  pthread_join(pid[0], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);
  pthread_join(pid[1], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);
  pthread_join(pid[2], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);

  icom_deinit(icomConnect[0]);
  icom_deinit(icomConnect[1]);
  icom_deinit(icomConnect[2]);
  icom_deinit(icomBind[0]);
  icom_deinit(icomBind[1]);
}

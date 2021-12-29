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

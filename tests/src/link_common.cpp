#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include "link_common.h"

extern "C" {
  #include "icom.h"
  #include "string_parser.h"
}

////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////
typedef struct {
  icom_t    *icom;
  void      *buf;
  unsigned   bufSize;
  unsigned   times;
} thread_send_t;


static void* thread_send(void *p){
  icomStatus_t status;
  thread_send_t *pdata = (thread_send_t*)p;

  for(int i=0; i<pdata->times; i++){
    status = icom_send(pdata->icom, pdata->buf, pdata->bufSize);
  }

  return (void*)status;
}


void link_common_initialization(const char *icomStr, unsigned testCount){
  icom_t *icom;
  for(unsigned i=0; i<testCount; i++){
    icom = icom_init(icomStr);
    EXPECT_FALSE(ICOM_IS_ERR(icom));
    icom_deinit(icom);
  }
}


////////////////////////////////////////////////////////////////////////////////
// GENERIC-TEST - SIMPLE TRANSFER
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
  thread_pdata = {icom_tx, txBuf, txBufSize, 1};
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
// GENERIC_TEST - VARIED TRANSFERS
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
    thread_pdata = {icom_tx, txBuf, txBufSize, 1};
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
// GENERIC_TEST - TOPOLOGY
////////////////////////////////////////////////////////////////////////////////
void link_common_topology(std::vector<const char*> icomRxStr, std::vector<const char*> icomTxStr, uint32_t transfers){
  std::vector<icom_t*> icom_rx;
  std::vector<icom_t*> icom_tx;
  std::vector<thread_send_t*> icom_tx_pdata;
  std::vector<pthread_t> icom_tx_pids;
  std::vector<const char*> tx_bufs;
  std::vector<const char*> rx_bufs;
  unsigned bytes_sent = 0, bytes_received = 0;
  icomStatus_t status;

  for(auto &str : icomRxStr){
    icom_rx.push_back(icom_init(str));
    EXPECT_FALSE(ICOM_IS_ERR(icom_rx.back()));
  }

  for(auto &str : icomTxStr){
    pthread_t pid;
    icom_tx.push_back(icom_init(str));
    EXPECT_FALSE(ICOM_IS_ERR(icom_tx.back()));

    unsigned connections = parser_getConnectionCount(str);
    for(unsigned i=0; i<connections; i++){
      tx_bufs.push_back(str);  // no need to duplicate, because resides in icomRxStr
      bytes_sent += strlen(str)+1;
    }

    thread_send_t *thread_pdata = new thread_send_t;
    *thread_pdata = {icom_tx.back(), (char*)str, (uint32_t)strlen(str)+1, transfers};
    icom_tx_pdata.push_back(thread_pdata);

    pthread_create(&pid, NULL, thread_send, icom_tx_pdata.back());
    icom_tx_pids.push_back(pid);
  }


  for(auto &icom : icom_rx){
    void *buf = NULL;
    unsigned size;

    status = icom_recv(icom);
    EXPECT_EQ(status, ICOM_SUCCESS);
    
    while(icom_nextBuffer(icom, &buf, &size)){
      bytes_received += size;
      rx_bufs.push_back(strdup((char*)buf));  // string must be duplicated, buffer may change
    }
  }

  /* sort buffers */
  std::sort(tx_bufs.begin(), tx_bufs.end(),
    [](const char *l, const char *r){
      return (strcmp(l,r) > 0);});
  std::sort(rx_bufs.begin(), rx_bufs.end(),
    [](const char *l, const char *r){
      return (strcmp(l,r) > 0);});

  #if 0
  std::cout << "Tx buffers:" << std::endl;
  for(auto &buf : tx_bufs){
    std::cout << buf << std::endl;
  }
  std::cout << "Rx buffers:" << std::endl;
  for(auto &buf : rx_bufs){
    std::cout << buf << std::endl;
  }
  #endif

  /* compare sent/received number of bytes */
  EXPECT_EQ(bytes_sent, bytes_received);

  /* compare sent/received data */
  EXPECT_TRUE(
    std::equal(tx_bufs.begin(), tx_bufs.end(), rx_bufs.begin(),
      [](const char *l, const char *r) {
        return (strcmp(l,r) == 0);}));

  /* release allocated memory */
  for(auto &buf : rx_bufs){
    free((void*)buf);
  }

  for(auto &pid : icom_tx_pids){
    void *ret;
    pthread_join(pid, &ret);
    EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);
  }

  while(!icom_tx_pdata.empty()){
    delete icom_tx_pdata.back();
    icom_tx_pdata.pop_back();
  }

  while(!icom_rx.empty()){
    icom_deinit(icom_rx.back());
    icom_rx.pop_back();
  }

  while(!icom_tx.empty()){
    icom_deinit(icom_tx.back());
    icom_tx.pop_back();
  }
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - TIMEOUT
////////////////////////////////////////////////////////////////////////////////
void link_common_timeout_rx(const char *icomRxStr, const char *icomTxStr){
  icom_t *icom_rx, *icom_tx;
  icomStatus_t status;
  uint8_t *rxBuf;
  uint32_t rxBufSize;
  uint8_t txBuf[] = {1,2,3,4};

  /* initialize communication objects */
  icom_rx = icom_init(icomRxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_rx));

  icom_tx = icom_init(icomTxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_tx));

  /* initiate send/receive processes */
  status = icom_recv(icom_rx, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_TIMEOUT);

  /* timeout is optional, because sending is buffered */
  status = icom_send(icom_rx, (void*)txBuf, sizeof(txBuf));
  EXPECT_TRUE(status == ICOM_TIMEOUT || status == ICOM_SUCCESS);

  /* cleanup */
  icom_deinit(icom_rx);
  icom_deinit(icom_tx);
}


void link_common_timeout_tx(const char *icomRxStr, const char *icomTxStr){
  icom_t *icom_rx, *icom_tx;
  icomStatus_t status;
  uint8_t *rxBuf;
  uint32_t rxBufSize;
  uint8_t txBuf[] = {1,2,3,4};

  /* initialize communication objects */
  icom_rx = icom_init(icomRxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_rx));

  icom_tx = icom_init(icomTxStr);
  EXPECT_FALSE(ICOM_IS_ERR(icom_tx));

  /* initiate send/receive processes */
  status = icom_recv(icom_tx, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_TIMEOUT);

  /* timeout is optional, because sending is buffered */
  status = icom_send(icom_tx, (void*)txBuf, sizeof(txBuf));
  EXPECT_TRUE(status == ICOM_TIMEOUT || status == ICOM_SUCCESS);

  /* cleanup */
  icom_deinit(icom_rx);
  icom_deinit(icom_tx);
}

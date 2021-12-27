#include <pthread.h>
#include "gtest/gtest.h"
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

void* thread_send(void *p){
  thread_send_t *pdata = (thread_send_t*)p;
  return (void*)icom_send(pdata->icom, pdata->buf, pdata->bufSize);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED
////////////////////////////////////////////////////////////////////////////////

TEST(link_socket, init_bind){
  icom_t *icomBind;
  icomBind = icom_init("socket_rx|default|*:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));
  icom_deinit(icomBind);
}

TEST(link_socket, init_connect){
  icom_t *icomConnect;
  icomConnect = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));
  icom_deinit(icomConnect);
}


TEST(link_socket, transfer_simple){
  icom_t *icomBind, *icomConnect;
  icomStatus_t status;
  thread_send_t thread_pdata;
  pthread_t pid;
  char *rxBuf, txBuf[4] = "abc";
  uint32_t rxBufSize, txBufSize = sizeof(txBuf);
  void *ret;

  icomConnect = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));
  icomBind = icom_init("socket_rx|default|*:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  thread_pdata = {icomConnect, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status,    ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_STREQ(rxBuf,  txBuf);

  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);
  icom_deinit(icomConnect);
  icom_deinit(icomBind);
}


TEST(link_socket, transfer_varying_size){
  icom_t *icomBind, *icomConnect;
  icomStatus_t status;
  thread_send_t thread_pdata;
  pthread_t pid;
  char *rxBuf, txBuf[4] = "abc";
  uint32_t rxBufSize, txBufSize = sizeof(txBuf);
  void *ret;

  icomConnect = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));
  icomBind = icom_init("socket_rx|default|*:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  /* Send 4 bytes */
  thread_pdata = {icomConnect, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status,    ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_STREQ(rxBuf,  txBuf);
  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);

  /* Send 3 bytes */
  txBuf[2] = '\0';
  thread_pdata = {icomConnect, txBuf, 3};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status,    ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, 3);
  EXPECT_STREQ(rxBuf,  txBuf);
  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);

  /* Send 2 bytes */
  txBuf[1] = '\0';
  thread_pdata = {icomConnect, txBuf, 2};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status,    ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, 2);
  EXPECT_STREQ(rxBuf,  txBuf);
  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);

  icom_deinit(icomConnect);
  icom_deinit(icomBind);
}


TEST(link_socket, transfer_10MB){
  icom_t *icomBind, *icomConnect;
  icomStatus_t status;
  thread_send_t thread_pdata;
  pthread_t pid;
  uint8_t *rxBuf, *txBuf;
  uint32_t rxBufSize, txBufSize = 10*1024*1024*sizeof(*txBuf);
  void *ret;

  icomConnect = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));
  icomBind = icom_init("socket_rx|default|*:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  txBuf = (uint8_t*)malloc(txBufSize);
  EXPECT_FALSE(txBuf == NULL);
  for(int i=0; i<txBufSize/sizeof(*txBuf); i++){
    txBuf[i] = i;
  }

  thread_pdata = {icomConnect, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);
  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_TRUE(memcmp(rxBuf, txBuf, txBufSize) == 0);
  pthread_join(pid, &ret);
  EXPECT_EQ((uint64_t)ret, ICOM_SUCCESS);

  icom_deinit(icomConnect);
  icom_deinit(icomBind);
}


TEST(link_socket, transfer_multiple){
  icom_t *icomBind, *icomConnect0, *icomConnect1;
  icomStatus_t status;
  thread_send_t thread_pdata0, thread_pdata1;
  pthread_t pid[2];
  uint8_t *rxBuf, *txBuf;
  uint32_t rxBufSize, txBufSize = 10*sizeof(*txBuf);
  void *ret;

  icomConnect0 = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect0));
  icomConnect1 = icom_init("socket_tx|default|127.0.0.1:8890");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect1));
  icomBind = icom_init("socket_rx|default|*:[8889-8890]");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  txBuf = (uint8_t*)malloc(txBufSize);
  EXPECT_FALSE(txBuf == NULL);
  for(int i=0; i<txBufSize/sizeof(*txBuf); i++){
    txBuf[i] = i;
  }

  thread_pdata0 = {icomConnect0, txBuf, txBufSize};
  pthread_create(&pid[0], NULL, thread_send, &thread_pdata0);
  thread_pdata1 = {icomConnect1, txBuf, txBufSize};
  pthread_create(&pid[1], NULL, thread_send, &thread_pdata1);

  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_EQ(memcmp(rxBuf, txBuf, txBufSize), 0);
  icom_nextBuffer(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize);
  EXPECT_EQ(memcmp(rxBuf, txBuf, txBufSize), 0);

  pthread_join(pid[0], &ret);
  EXPECT_TRUE((uint64_t)ret == ICOM_SUCCESS);
  pthread_join(pid[1], &ret);
  EXPECT_TRUE((uint64_t)ret == ICOM_SUCCESS);

  icom_deinit(icomConnect0);
  icom_deinit(icomConnect1);
  icom_deinit(icomBind);
}


TEST(link_socket, transfer_complex){
  icom_t *icomBind0, *icomBind1, *icomConnect0, *icomConnect1, *icomConnect2;
  icomStatus_t status;
  thread_send_t thread_pdata0, thread_pdata1, thread_pdata2;
  pthread_t pid[3];
  char *rxBuf; uint32_t rxBufSize;
  const char *txBuf0 = "buffer0"; uint32_t txBufSize0 = sizeof(txBuf0);
  const char *txBuf1 = "buffer1"; uint32_t txBufSize1 = sizeof(txBuf1);
  const char *txBuf2 = "buffer2"; uint32_t txBufSize2 = sizeof(txBuf2);
  void *ret;

  icomConnect0 = icom_init("socket_tx|default|127.0.0.1:8889");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect0));
  icomConnect1 = icom_init("socket_tx|default|127.0.0.1:[8890-8891]");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect1));
  icomConnect2 = icom_init("socket_tx|default|127.0.0.1:8892");
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect2));
  icomBind0 = icom_init("socket_rx|default|*:[8889-8890]");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind0));
  icomBind1 = icom_init("socket_rx|default|*:[8891-8892]");
  EXPECT_FALSE(ICOM_IS_ERR(icomBind1));

  thread_pdata0 = {icomConnect0, (char*)txBuf0, txBufSize0};
  pthread_create(&pid[0], NULL, thread_send, &thread_pdata0);
  thread_pdata1 = {icomConnect1, (char*)txBuf1, txBufSize1};
  pthread_create(&pid[1], NULL, thread_send, &thread_pdata1);
  thread_pdata2 = {icomConnect2, (char*)txBuf2, txBufSize2};
  pthread_create(&pid[2], NULL, thread_send, &thread_pdata2);

  status = icom_recv(icomBind0, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize0);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf0, txBufSize0) == 0)
            || (memcmp(rxBuf, txBuf1, txBufSize1) == 0));

  icom_nextBuffer(icomBind0, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize1);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf0, txBufSize0) == 0)
            || (memcmp(rxBuf, txBuf1, txBufSize1) == 0));

  status = icom_recv(icomBind1, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize1);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf1, txBufSize1) == 0)
            || (memcmp(rxBuf, txBuf2, txBufSize2) == 0));

  icom_nextBuffer(icomBind1, (void**)&rxBuf, &rxBufSize);
  EXPECT_EQ(status, ICOM_SUCCESS);
  EXPECT_EQ(rxBufSize, txBufSize2);
  EXPECT_TRUE( (memcmp(rxBuf, txBuf1, txBufSize1) == 0)
            || (memcmp(rxBuf, txBuf2, txBufSize2) == 0));

  pthread_join(pid[0], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);
  pthread_join(pid[1], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);
  pthread_join(pid[2], &ret);
  EXPECT_EQ((icomStatus_t)(uint64_t)ret, ICOM_SUCCESS);

  icom_deinit(icomConnect0);
  icom_deinit(icomConnect1);
  icom_deinit(icomConnect2);
  icom_deinit(icomBind0);
  icom_deinit(icomBind1);
}


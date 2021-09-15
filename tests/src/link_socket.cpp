#include <pthread.h>
#include "gtest/gtest.h"
extern "C" {
  #include "icom.h"
}

TEST(link_socket, init_bind){
  icom_t *icomBind;

  icomBind = icom_init("socket_rx:*:8889", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  icom_deinit(icomBind);
}


TEST(link_socket, init_connect){
  icom_t *icomConnect;

  icomConnect = icom_init("socket_tx:127.0.0.1:8889", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));

  icom_deinit(icomConnect);
}


typedef struct {
  icom_t    *icom;
  void      *buf;
  unsigned   bufSize;
} thread_send_t;

void* thread_send(void *p){
  thread_send_t *pdata = (thread_send_t*)p;
  return (void*)icom_send(pdata->icom, pdata->buf, pdata->bufSize);
}


TEST(link_socket, send_recv_simple){
  icom_t *icomBind, *icomConnect;
  icomStatus_t status;
  thread_send_t thread_pdata;
  pthread_t pid;
  char *rxBuf, txBuf[4] = "abc";
  uint32_t rxBufSize, txBufSize = sizeof(txBuf);
  void *pthreadRet;

  icomConnect = icom_init("socket_tx:127.0.0.1:8890", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));

  icomBind = icom_init("socket_rx:*:8890", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  thread_pdata = {icomConnect, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);

  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(rxBufSize == txBufSize);
  EXPECT_STREQ(rxBuf, txBuf);

  pthread_join(pid, &pthreadRet);
  EXPECT_TRUE((icomStatus_t)(unsigned long long)pthreadRet == ICOM_SUCCESS);

  icom_deinit(icomConnect);
  icom_deinit(icomBind);
}


TEST(link_socket, send_recv_varying){
  icom_t *icomBind, *icomConnect;
  icomStatus_t status;
  thread_send_t thread_pdata;
  pthread_t pid;
  char *rxBuf, txBuf[4] = "abc";
  uint32_t rxBufSize, txBufSize = sizeof(txBuf);
  void *pthreadRet;

  icomConnect = icom_init("socket_tx:127.0.0.1:8890", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomConnect));

  icomBind = icom_init("socket_rx:*:8890", 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));


  thread_pdata = {icomConnect, txBuf, txBufSize};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);

  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(rxBufSize == txBufSize);
  EXPECT_STREQ(rxBuf, txBuf);

  pthread_join(pid, &pthreadRet);
  EXPECT_TRUE((icomStatus_t)(unsigned long long)pthreadRet == ICOM_SUCCESS);


  txBuf[2] = '\0';
  thread_pdata = {icomConnect, txBuf, 2};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);

  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(rxBufSize == 2);
  EXPECT_STREQ(rxBuf, txBuf);

  pthread_join(pid, &pthreadRet);
  EXPECT_TRUE((icomStatus_t)(unsigned long long)pthreadRet == ICOM_SUCCESS);


  txBuf[1] = '\0';
  thread_pdata = {icomConnect, txBuf, 1};
  pthread_create(&pid, NULL, thread_send, &thread_pdata);

  status = icom_recv(icomBind, (void**)&rxBuf, &rxBufSize);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(rxBufSize == 1);
  EXPECT_STREQ(rxBuf, txBuf);

  pthread_join(pid, &pthreadRet);
  EXPECT_TRUE((icomStatus_t)(unsigned long long)pthreadRet == ICOM_SUCCESS);

  icom_deinit(icomConnect);
  icom_deinit(icomBind);
}

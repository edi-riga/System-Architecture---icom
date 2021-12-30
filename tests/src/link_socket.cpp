#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "link_common.h"

extern "C" {
  #include "icom.h"
}

#define INIT_TEST_COUNT 100

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
// TEST-RELATED - INITIALIZATION/DEINITIALIZATION
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, init_rx_default){
  link_common_initialization("socket_rx|default|*:8889", INIT_TEST_COUNT);
}

TEST(link_socket, init_tx_default){
  link_common_initialization("socket_tx|default|127.0.0.1:8889", INIT_TEST_COUNT);
}

TEST(link_socket, init_rx_zero){
  link_common_initialization("socket_rx|zero|*:8889", INIT_TEST_COUNT);
}

TEST(link_socket, init_tx_zero){
  link_common_initialization("socket_tx|zero|127.0.0.1:8889", INIT_TEST_COUNT);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - SIMPLE TRANSFER
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_simple_default){
  link_common_simple(
    "socket_tx|default|127.0.0.1:8889",
    "socket_rx|default|*:8889",
    12); // size in bytes
}

TEST(link_socket, transfer_simple_zero){
  link_common_simple(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    12); // size in bytes
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - VARIED TRANSFERS
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_varied_default){
  link_common_varied(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    100); // TEST_COUNT
}

TEST(link_socket, transfer_varied_zero){
  link_common_varied(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    100); // TEST_COUNT
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - LARGE TRANSFERS
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_100Mb_default){
  link_common_simple(
    "socket_tx|default|127.0.0.1:8889",
    "socket_rx|default|*:8889",
    100*1024*1024); // size in bytes
}

TEST(link_socket, transfer_100Mb_zero){
  link_common_simple(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    100*1024*1024); // size in bytes
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - FAN-IN COMMUNICATION
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_fanin_default){
  const char *connectStrings[] = {
    "socket_tx|default|127.0.0.1:8889", 
    "socket_tx|default|127.0.0.1:8890", 
    "socket_tx|default|127.0.0.1:8891", 
  };
  const char *bindString = "socket_rx|default|*:[8889-8891]";
  link_common_fanin(connectStrings, bindString, 3);
}

TEST(link_socket, transfer_fanin_zero){
  const char *connectStrings[] = {
    "socket_tx|zero|127.0.0.1:8889", 
    "socket_tx|zero|127.0.0.1:8890", 
    "socket_tx|zero|127.0.0.1:8891", 
  };
  const char *bindString = "socket_rx|zero|*:[8889-8891]";
  link_common_fanin(connectStrings, bindString, 3);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - COMPLEX
////////////////////////////////////////////////////////////////////////////////
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


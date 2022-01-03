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
  const char *connectStrings[] = {
    "socket_tx|default|127.0.0.1:8889",
    "socket_tx|default|127.0.0.1:[8890-8891]",
    "socket_tx|default|127.0.0.1:8892"};
  const char *bindStrings[] = {
    "socket_rx|default|*:[8889-8890]",
    "socket_rx|default|*:[8891-8892]"};
  link_common_complex(connectStrings, bindStrings);
}

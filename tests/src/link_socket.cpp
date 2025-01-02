#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
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
TEST(link_socket, init_rx_zero){
  link_common_initialization("socket_rx|zero|*:8889", INIT_TEST_COUNT);
}
TEST(link_socket, init_rx_timeout){
  link_common_initialization("socket_rx|timeout|*:8889", INIT_TEST_COUNT);
}
TEST(link_socket, init_rx_zero_timeout){
  link_common_initialization("socket_rx|zero,timeout|*:8889", INIT_TEST_COUNT);
}

TEST(link_socket, init_tx_default){
  link_common_initialization("socket_tx|default|127.0.0.1:8889", INIT_TEST_COUNT);
}
TEST(link_socket, init_tx_zero){
  link_common_initialization("socket_tx|zero|127.0.0.1:8889", INIT_TEST_COUNT);
}
TEST(link_socket, init_tx_timeout){
  link_common_initialization("socket_tx|timeout|127.0.0.1:8889", INIT_TEST_COUNT);
}
TEST(link_socket, init_tx_zero_timeout){
  link_common_initialization("socket_tx|zero,timeout|127.0.0.1:8889", INIT_TEST_COUNT);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - SIMPLE TRANSFER
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_simple_default){
  for(uint32_t size=4; size<12; size++){
    link_common_simple(
      "socket_tx|default|127.0.0.1:8889",
      "socket_rx|default|*:8889",
      size);
  }
}
TEST(link_socket, transfer_simple_default_reverse){
  for(uint32_t size=4; size<12; size++){
    link_common_simple(
      "socket_tx|default|127.0.0.1:8889",
      "socket_rx|default|*:8889",
      size);
  }
}

TEST(link_socket, transfer_simple_zero){
  for(uint32_t size=4; size<12; size++){
    link_common_simple(
      "socket_tx|zero|127.0.0.1:8889",
      "socket_rx|zero|*:8889",
      size); // size in bytes
  }
}
//TEST(link_socket, transfer_simple_zero_reverse){
//  link_common_simple(
//    "socket_rx|zero|*:8889",
//    "socket_tx|zero|127.0.0.1:8889",
//    12); // size in bytes
//}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - VARIED TRANSFERS
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_varied_default){
  link_common_varied(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    100);
}

TEST(link_socket, transfer_varied_zero){
  link_common_varied(
    "socket_tx|zero|127.0.0.1:8889",
    "socket_rx|zero|*:8889",
    100);
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
std::vector<const char*> icomRxStr_fanin_default{
  "socket_rx|default|*:[8889-8891]"
};
std::vector<const char*> icomTxStr_fanin_default{
  "socket_tx|default|127.0.0.1:8889", 
  "socket_tx|default|127.0.0.1:8890", 
  "socket_tx|default|127.0.0.1:8891", 
};

std::vector<const char*> icomRxStr_fanin_zero{
  "socket_rx|zero|*:[8889-8891]"
};
std::vector<const char*> icomTxStr_fanin_zero{
  "socket_tx|zero|127.0.0.1:8889", 
  "socket_tx|zero|127.0.0.1:8890", 
  "socket_tx|zero|127.0.0.1:8891", 
};


TEST(link_socket, transfer_fanin_default_1x){
  link_common_topology(icomRxStr_fanin_default, icomTxStr_fanin_default, 1);
}
TEST(link_socket, transfer_fanin_default_10000x){
  link_common_topology(icomRxStr_fanin_default, icomTxStr_fanin_default, 10000);
}
TEST(link_socket, transfer_fanin_zero_1x){
  link_common_topology(icomRxStr_fanin_default, icomTxStr_fanin_default, 1);
}
TEST(link_socket, transfer_fanin_zero_10000x){
  link_common_topology(icomRxStr_fanin_default, icomTxStr_fanin_default, 10000);
}


////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - COMPLEX
////////////////////////////////////////////////////////////////////////////////
std::vector<const char*> icomTxStr_complex_default{
  "socket_tx|default|127.0.0.1:8889",
  "socket_tx|default|127.0.0.1:[8890-8891]",
  "socket_tx|default|127.0.0.1:8892",
};
std::vector<const char*> icomRxStr_complex_default{
    "socket_rx|default|*:[8889-8890]",
    "socket_rx|default|*:[8891-8892]",
};


TEST(link_socket, transfer_complex_1x){
  link_common_topology(icomRxStr_complex_default, icomTxStr_complex_default, 1);
}
TEST(link_socket, transfer_complex_10000x){
  link_common_topology(icomRxStr_complex_default, icomTxStr_complex_default, 10000);
}

////////////////////////////////////////////////////////////////////////////////
// TEST-RELATED - TIMEOUT
////////////////////////////////////////////////////////////////////////////////
TEST(link_socket, transfer_timeout_rx){
  const char *bindString    = "socket_rx|timeout|*:8889";
  const char *connectString = "socket_tx|timeout|127.0.0.1:8889";
  link_common_timeout_rx(bindString, connectString);
}

TEST(link_socket, transfer_timeout_tx){
  const char *bindString    = "socket_rx|timeout|*:8889";
  const char *connectString = "socket_tx|timeout|127.0.0.1:8889";
  link_common_timeout_tx(bindString, connectString);
}

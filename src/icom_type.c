#include "icom_type.h"

const char* icomTypeStrings[] = {
  "ICOM_TYPE_SOCKET_CONNECT",
  "ICOM_TYPE_SOCKET_BIND",
  "ICOM_TYPE_FIFO",
  "ICOM_TYPE_ZMQ_PUSH",
  "ICOM_TYPE_ZMQ_PULL",
  "ICOM_TYPE_ZMQ_PUB",
  "ICOM_TYPE_ZMQ_SUB",
  "ICOM_TYPE_ZMQ_REQ",
  "ICOM_TYPE_ZMQ_REP",
  "ICOM_TYPE_AUTO",
};


const char* icom_typeToString(icomType_t type){
  if(type > ICOM_TYPE_AUTO){
    return "INVALID TYPE";
  }

  return icomTypeStrings[type];
}

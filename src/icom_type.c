#include <string.h>
#include "icom_type.h"

const char* icomTypeStrings[] = {
  "socket_tx",
  "socket_rx",
  "fifo_tx",
  "fifo_rx",
  "zmq_push",
  "zmq_pull",
  "zmq_pub",
  "zmq_sub",
  "zmq_req",
  "zmq_rep",
  "auto",
};


const char* icom_typeToString(icomType_t type){
  if(type > ICOM_TYPE_AUTO){
    return "INVALID TYPE";
  }

  return icomTypeStrings[type];
}

icomType_t icom_stringToType(const char *typeString){
  for(int i=0; i<sizeof(icomTypeStrings)/sizeof(*icomTypeStrings); i++){
    if(strcmp(typeString, icomTypeStrings[i]) == 0){
      return i; // corresponds to enum (in icom_type.h)
    }
  }

  return ICOM_TYPE_NONE;
}

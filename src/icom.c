#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

#include "config.h"
#include "macro.h"
#include "notification.h"
#include "string_parser.h"

#include "link_zmq.h"
#include "link_fifo.h"
#include "link_socket.h"


icomLink_t* (*icomInitHandlers[])(const char*, icomType_t, icomFlags_t) = {
  icom_initSocketConnect,
  icom_initSocketBind,
  icom_initFifo,
  icom_initZmqPush,
  icom_initZmqPull,
  icom_initZmqPub,
  icom_initZmqSub,
  icom_initZmqReq,
  icom_initZmqRep,
};

void (*icomDeinitHandlers[])(icomLink_t*) = {
  icom_deinitSocket,
  icom_deinitSocket,
  icom_deinitFifo,
  icom_deinitZmqPush,
  icom_deinitZmqPull,
  icom_deinitZmqPub,
  icom_deinitZmqSub,
  icom_deinitZmqReq,
  icom_deinitZmqRep,
};


icomStatus_t retreiveComType(const char *comString, icomType_t *type){
  char typeString[MAX_TYPE_STRING_LENGTH+1];

  /* retreive communication type string */
  int r = sscanf(comString, "%" STR(MAX_TYPE_STRING_LENGTH)  "[^:]:", typeString);
  if(r != 1){
    _E("Failed to parse communication type string, %d", r);
    return ICOM_EINVAL;
  }

  /* retreive the actual communication type */
  *type = icom_stringToType(typeString);
  if(*type == ICOM_TYPE_NONE){
    _E("Failed to lookup communication type");
    return ICOM_ELOOKUP;
  }

  return ICOM_SUCCESS;
}

icomLink_t* icom_initGeneric(const char *comString, icomFlags_t flags){
  icomLink_t* connection;
  icomType_t type;
  icomStatus_t status;

  /* retreive communication type */
  status = retreiveComType(comString, &type);
  if(status != ICOM_SUCCESS){
    _E("Failed to retreive communication type");
    return (icomLink_t*)status;
  }

  _D("%u (%s) type requested", type, icom_typeToString(type));

  if(type > ICOM_TYPE_AUTO){
    _E("Invalid icom type");
    return (icomLink_t*)ICOM_ITYPE;
  }

  if(type < ICOM_TYPE_AUTO){
    return icomInitHandlers[type](comString, type, flags);
  }

  // otherwise a straightforward trial and error method (should work until zmq)
  for(int i=0; i<ICOM_TYPE_AUTO; i++){
    connection = icomInitHandlers[i](comString, type, flags);
    if(!ICOM_IS_ERR(connection)){
      return connection;
    }
  }

  return (icomLink_t*)ICOM_ERROR;

  //{ // ICOM_TYPE_NATIVE_SOCKET: "*.*.*.*:*"
  //  uint8_t  ip4[4];
  //  uint16_t port;
  //  ret = sscanf(comString, "%hhu.%hhu.%hhu.%hhu:%hu", &ip4[4], &ip4[2], &ip4[1], &ip4[0], &port);
  //  if(ret == 5){
  //    return icom_initNativeSocket(ip4, port, flags);
  //  }
  //}

  //{ // ICOM_TYPE_ZMQ_PUSH:  "tcp://*:*" || "inproc://*"
  //  if( strstr(comString, "tcp:") == comString || strstr(comString, "inproc:") == comString){
  //  }
  //}

  //{ // ICOM_TYPE_ZMQ_PULL
  //}
}

void icom_deinitGeneric(icomLink_t* connection){
}


icom_t* icom_init(const char *comString, icomFlags_t flags){
  int i;
  int r;
  icom_t *icom, *ret;

  /* allocate and initialize icom structure */
  icom = (icom_t*)malloc(sizeof(icom_t));
  if(!icom){
    _E("Failed to allocate memory");
    return (icom_t*)ICOM_ENOMEM;
  }

  /* parse communication strings */
  r = parser_initStrArray(&icom->comStrings, &icom->comCount, comString);
  if(r != 0){
    _E("Failed to parse communication string");
    ret = (icom_t*)ICOM_EINVAL;
    goto failure_initStrArray;
  }

  /* allocate memory for connection struct array */
  icom->comConnections = (icomLink_t**)malloc(icom->comCount*sizeof(icomLink_t*));
  if(!icom->comConnections){
    _E("Failed to allocate memory");
    ret = (icom_t*)ICOM_ENOMEM;
    goto failure_malloc_connections;
  }

  /* initialize selected icom communication */
  for(i=0; i<icom->comCount; i++){
    icom->comConnections[i] = icom_initGeneric(icom->comStrings[i], flags);
    if( ICOM_IS_ERR(icom->comConnections[i]) ){
      _E("Failed to initialize connection: %s", icom->comStrings[i]);
      ret = (icom_t*)icom->comConnections[i];
      goto failure_initGeneric;
    }
  }

  return icom;


failure_initGeneric:
  for(--i; i>=0; i--){
    icom_deinitGeneric(icom->comConnections[i]);
  }
failure_malloc_connections:
  parser_deinitStrArray(icom->comStrings, icom->comCount);
failure_initStrArray:
  free(icom);
  return ret;
}


void icom_deinit(icom_t* icom){
  int i;

  /* deallocate connection objects */
  for(i=0; i<icom->comCount; i++){
    icom_deinitGeneric(icom->comConnections[i]);
  }

  /* deallocate connection array */
  free(icom->comConnections);

  /* free communication strings */
  parser_deinitStrArray(icom->comStrings, icom->comCount);

  /* deallocate icom structure  */
  free(icom);
}


icomStatus_t icom_send(icom_t *icom, void  *buf, unsigned bufSize){
  icomStatus_t status[icom->comCount];

  for(int i=0; i<icom->comCount; i++){
    status[i] = icom->comConnections[i]->sendHandler(icom->comConnections[i], buf, bufSize);
  }

  /* TODO: analyze return values */

  return status[0];
}

icomStatus_t icom_recv(icom_t *icom, void **buf, unsigned *bufSize){
  icomStatus_t status[icom->comCount];

  for(int i=0; i<icom->comCount; i++){
    status[i] = icom->comConnections[i]->recvHandler(icom->comConnections[i], buf, bufSize);
  }

  /* TODO: analyze return values */

  return status[0];
}

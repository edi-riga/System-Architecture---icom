#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_flags.h"
#include "icom_status.h"

#include "config.h"
#include "macro.h"
#include "notification.h"
#include "string_parser.h"

#include "link_zmq.h"
#include "link_fifo.h"
#include "link_socket.h"


icomStatus_t (*icomInitHandlers[])(icomLink_t*, icomType_t, const char*, icomFlags_t) = {
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

icomStatus_t icom_initGeneric(icomLink_t *connection, icomType_t type, const char *comString, icomFlags_t flags){
  icomStatus_t status;

  _D("%u (%s) type requested with flags - %u (%s) - and communication string: \"%s\"",
    type, icom_typeToString(type),
    flags, icom_flagsToString(flags),
    comString);

  if(type > ICOM_TYPE_AUTO){
    _E("Invalid icom type");
    return ICOM_ITYPE;
  }

  if(type < ICOM_TYPE_AUTO){
    return icomInitHandlers[type](connection, type, comString, flags);
  }

  /* otherwise a straightforward trial and error method (should work until zmq) */
  for(int i=0; i<ICOM_TYPE_AUTO; i++){
    status = icomInitHandlers[i](connection, type, comString, flags);
    if(status != ICOM_SUCCESS){
      return ICOM_SUCCESS;
    }
  }

  return ICOM_ERROR;
}

void icom_deinitGeneric(icomLink_t* connection){
  icomDeinitHandlers[connection->type](connection);
}


icom_t* icom_init(const char *comString){
  char **fieldArray; uint32_t fieldCount;
  icomType_t comType; icomFlags_t comFlags;
  int i;
  int r;
  icomStatus_t status;
  icom_t *icom, *ret;

  /* allocate and initialize icom structure */
  icom = (icom_t*)malloc(sizeof(icom_t));
  if(!icom){
    _E("Failed to allocate memory");
    return (icom_t*)ICOM_ENOMEM;
  }

  /* parse fields in the communication string */
  r = parser_initFields(&fieldArray, &fieldCount, comString, ICOM_DELIMITER);
  if(r != 0){
    _E("Failed to parse communication string");
    ret = (icom_t*)ICOM_EINVAL;
    goto failure_initFields;
  }

  for(i=0; i<fieldCount; i++){
  _D("Field %d in communication string: %s", i, fieldArray[i]);
  }

  /* get communication type (1st field string) */
  comType = icom_stringToType(fieldArray[0]);
  if(comType == ICOM_TYPE_NONE){
    _E("Invalid configuration");
    ret = (icom_t*)ICOM_ELOOKUP;
    goto failure_getType;
  }

  /* get communication flags */
  comFlags = icom_stringToFlags(fieldArray[1]);
  if(comType & ICOM_FLAG_INVALID){
    _E("Invalid configuration");
    ret = (icom_t*)ICOM_ELOOKUP;
    goto failure_getFlags;
  }

  /* parse communication strings */
  r = parser_initStrArray(&icom->comStrings, &icom->comCount, fieldArray[2]);
  if(r != 0){
    _E("Failed to parse communication string");
    ret = (icom_t*)ICOM_EINVAL;
    goto failure_initStrArray;
  }

  /* allocate memory for connection struct array */
  icom->comConnections = (icomLink_t*)malloc(icom->comCount*sizeof(icomLink_t));
  if(!icom->comConnections){
    _E("Failed to allocate memory");
    ret = (icom_t*)ICOM_ENOMEM;
    goto failure_malloc_connections;
  }

  /* initialize selected icom communication */
  for(i=0; i<icom->comCount; i++){
    status = icom_initGeneric(&(icom->comConnections[i]), comType, icom->comStrings[i], comFlags);
    if( status != ICOM_SUCCESS ){
      _E("Failed to initialize connection: %s", icom->comStrings[i]);
      ret = (icom_t*)status;
      goto failure_initGeneric;
    }
  }

  /* deallocate communication fields (we don't need them) */
  parser_deinitFields(fieldArray, fieldCount);

  return icom;


failure_initGeneric:
  for(--i; i>=0; i--){
    icom_deinitGeneric(&(icom->comConnections[i]));
  }
  free(icom->comConnections);
failure_malloc_connections:
  parser_deinitStrArray(icom->comStrings, icom->comCount);
failure_initStrArray:
failure_getFlags:
failure_getType:
  parser_deinitFields(fieldArray, fieldCount);
failure_initFields:
  free(icom);
  return ret;
}


void icom_deinit(icom_t* icom){
  int i;

  /* deallocate connection objects */
  for(i=0; i<icom->comCount; i++){
    icom_deinitGeneric(&(icom->comConnections[i]));
  }

  /* deallocate connection array */
  free(icom->comConnections);

  /* deallocate communication strings */
  parser_deinitStrArray(icom->comStrings, icom->comCount);

  /* deallocate icom structure  */
  free(icom);
}


icomStatus_t icom_send(icom_t *icom, void  *buf, unsigned bufSize){
  icomStatus_t status[icom->comCount];

  for(int i=0; i<icom->comCount; i++){
    status[i] = icom->comConnections[i].sendHandler(icom->comConnections+i, buf, bufSize);
  }

  /* TODO: analyze return values */

  return status[0];
}

icomStatus_t icom_recv1(icom_t *icom){ //, void **buf, unsigned *bufSize){
  icomStatus_t status[icom->comCount];
  void *dummyBuf;

  for(int i=0; i<icom->comCount; i++){
    status[i] = icom->comConnections[i].recvHandler(
      icom->comConnections+i,
      &dummyBuf,
      &(icom->comConnections[i].recvBufSize));
  }

  /* TODO: analyze return values */

  return status[0];
}


icomStatus_t icom_recv3(icom_t *icom, void **buf, unsigned *bufSize){
  icomStatus_t status[icom->comCount];

  /* Perform all the data receptions in reverse order. The order is reveresed
   * so that the first buffer in the recvBuf list is returned and, therefore,
   * the icom_nextBuffer routine indeed would return the next buffer. */
  for(int i=icom->comCount-1; i>=0; i--){
    status[i] = icom->comConnections[i].recvHandler(icom->comConnections+i, buf, bufSize);
  }

  /* TODO: analyze return values */

  return status[0];
}


void* icom_nextBuffer(icom_t *icom, void **buf, unsigned *bufSize){
  icomLink_t *link;
  int bufIndex;

  /* NULL signals a request for the first buffer, there is an ugly workaround
   * for zero copy, i.e. if that's the case, we return pointer at the location
   * of the buffer */
  if(*buf == NULL){
    *bufSize = icom->comConnections[0].recvBufSize;
    *buf     = (icom->comConnections[0].flags & ICOM_FLAG_ZERO)
      ? *(void**)icom->comConnections[0].recvBuf
      :          icom->comConnections[0].recvBuf;
    return *buf;
  }

  /* Get ready for some sad (and probably dumb) pointer magic, but at the moment
   * I could not think of anything better :( Imporantly, for zero-copy use case,
   * we cannot determine the buffer's link. Firstly, note that normal buffers
   * have reference to their respective link just before the buffer address,
   * which is not the case for zero-copy buffers. So, firstly we try to identify
   * if the requested buffer's potential/theoretical link address indeed hits
   * the link array address region, if that is not the case, we assume that we
   * have been supplied with a zero-copy buffer. Further, we in a brute force
   * manner find the corresponding bufferIndex and return "next" buffer, hence
   * the (index+1) */
  if( (*(void**)(*buf-sizeof(icomLink_t*)) < (void*)(icom->comConnections))
  ||  (*(void**)(*buf-sizeof(icomLink_t*)) > (void*)(icom->comConnections + icom->comCount))){
    for(int bufIndex=0; bufIndex<icom->comCount-1; bufIndex++){
      if(*buf == *(void**)icom->comConnections[bufIndex].recvBuf){
        *bufSize = icom->comConnections[bufIndex+1].recvBufSize;
        *buf     = *(void**)icom->comConnections[bufIndex+1].recvBuf;
        return *buf;
      }
    }

    return NULL;
  }

  /* Non-NULL buffer value assumes request for the next buffer (1) get address
   * for the current buffer's link; (2) get the index of the link in the
   * connection list and increment it (try to get the "next" buffer) */
  link = *(icomLink_t**)(*buf-sizeof(link));
  bufIndex = (link-icom->comConnections) + 1;

  /* return "next" buffer if the index is valid */
  if(bufIndex < icom->comCount){
    *bufSize = icom->comConnections[bufIndex].recvBufSize;
    *buf     = icom->comConnections[bufIndex].recvBuf;
    return *buf;
  }

  return NULL;
}

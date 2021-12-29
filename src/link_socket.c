#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_socket.h"
#include "notification.h"
#include "config.h"
#include "macro.h"


static icomStatus_t link_sendHeader(icomLinkSocket_t *pdata, icomMsgHeader_t *header){
  if(send(pdata->fd, header, sizeof(*header), 0) == -1){
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }
  return ICOM_SUCCESS;
}


static icomStatus_t link_sendData(icomLinkSocket_t *pdata, void *buf, unsigned bufSize){
  int ret;
  _D("Sending data from %p (%u bytes)", buf, bufSize);

  /* Send data */
  ret = send(pdata->fd, buf, bufSize, 0);
  if(ret == -1){
    _SE("Send failed (data)");
    return ICOM_ERROR;
  }

  /* Check if size of requested and sent data is equal */
  if(ret != bufSize){
    return ICOM_PARTIAL;
  }

  return ICOM_SUCCESS;
}


static icomStatus_t link_sendDefault(icomLink_t *link, void *buf, unsigned bufSize){
  icomMsgHeader_t header;
  _D("Sending (Default handler)");

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Construct and send header */
  header = (icomMsgHeader_t){ICOM_TYPE_SOCKET_TX, link->flags, bufSize};
  if(link_sendHeader(pdata, &header) != ICOM_SUCCESS){
    return ICOM_ERROR;
  }

  /* Send data */
  return link_sendData(pdata, buf, bufSize);
}


static icomStatus_t link_sendZero(icomLink_t *link, void *buf, unsigned bufSize){
  icomMsgHeader_t header;
  _D("Sending (Zero handler)");

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Construct and send header */
  header = (icomMsgHeader_t){ICOM_TYPE_SOCKET_TX, link->flags, bufSize};
  if(link_sendHeader(pdata, &header) != ICOM_SUCCESS){
    return ICOM_ERROR;
  }

  /* Send data */
  return link_sendData(pdata, &buf, sizeof(void*));
}


static icomStatus_t link_recv(icomLink_t *link, void **buf,  unsigned *bufSize){
  icomMsgHeader_t header;
  int bytesReceived = 0;
  int ret;

  _D("Receiving at link: %p", link);

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Receive header */
  ret = recv(pdata->fdAccepted, &header, sizeof(header), 0);
  if(ret == -1){
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }

  /* Reallocate input buffer */
  if(link->recvBufSize != header.bufSize){
    link->recvBufSize = (header.flags & ICOM_FLAG_ZERO) ? sizeof(void*) : header.bufSize;
    link->recvBuf     = (void*)realloc(link->recvBuf-sizeof(link), sizeof(link) + link->recvBufSize);
    link->recvBuf    += sizeof(link);
  }

  /* Receive the actual data (which can be split into multiple messages) */
  do{
    ret = recv(pdata->fdAccepted, (uint8_t*)(link->recvBuf)+bytesReceived, link->recvBufSize-bytesReceived, 0);
    if(ret == -1){
      _SE("Send failed (header)");
      return ICOM_ERROR;
    }

    bytesReceived += ret;
  } while( (ret != -1) && (bytesReceived != link->recvBufSize));

  /* Setup output arguments */
  *buf     = (header.flags & ICOM_FLAG_ZERO) ? *(void**)link->recvBuf : link->recvBuf;
  *bufSize = (header.flags & ICOM_FLAG_ZERO) ? header.bufSize         : bytesReceived;

  /* Check if size of requested and sent data is equal */
  if(bytesReceived != link->recvBufSize){
    _W("Received partial data (%d/%d)", ret, link->recvBufSize);
    return ICOM_PARTIAL;
  }

  _D("Link at %p received %u bytes", link, *bufSize);

  return ICOM_SUCCESS;
}


static icomStatus_t link_connectAndSend(icomLink_t *link, void *buf, unsigned bufSize){

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  _D();

  /* Connect to the */
  if(connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to connect socket");
    if(errno == ECONNREFUSED){
      return ICOM_ECONNREFUSED;
    } else {
      return ICOM_ERROR;
    }
  }

  /* Assuming we are connected, switch future handler to a connected one */
  link->sendHandler = link->sendHandlerSecondary;

  /* Call send handler */
  return link->sendHandler(link, buf, bufSize);
}

static icomStatus_t link_acceptAndRecv(icomLink_t *link, void **buf, unsigned *bufSize){

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  _D();

  /* Connect to the */
  pdata->fdAccepted = accept(pdata->fd, NULL, NULL);
  if(pdata->fdAccepted == -1){
    _SE("Failed to accept socket");
    return ICOM_ERROR;
  }

  /* Assuming we have an accepted socket, switch future handler to just recv */
  link->recvHandler = link_recv;

  /* Call send handler */
  return link->recvHandler(link, buf, bufSize);
}


static icomStatus_t link_sendNotImplemented(icomLink_t *link, void *buf, unsigned bufSize){
  return ICOM_NIMPL;
}

static icomStatus_t link_recvNotImplemented(icomLink_t *link, void **buf, unsigned *bufSize){
  return ICOM_NIMPL;
}


icomStatus_t icom_initSocketConnect(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  icomStatus_t ret;
  icomLinkSocket_t *pdata;
  uint16_t port;
  const char *p;
  char ip[sizeof("xxx.xxx.xxx.xxx")];
  int not_connected = 0;

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%15[^:]:%hu", ip, &port);
  if(r != 2){
    _E("Failed to parse communication string (%d)", r);
    return ICOM_EINVAL;
  }

  /* allocating memory for the private link data structure */
  pdata = (icomLinkSocket_t*)malloc(sizeof(icomLinkSocket_t));
  if(!pdata){
    _E("Failed to allocate memory");
    return ICOM_ENOMEM;
  }

  /* creating TCP socket */
  pdata->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(pdata->fd == -1){
    _SE("Failed to create socket");
    ret = (icomStatus_t)errno;
    goto failure_socket;
  }

  /* set sender socket */
  memset(&pdata->sockaddr, 0, sizeof(struct sockaddr_in));
  pdata->sockaddr.sin_family = AF_INET;
  pdata->sockaddr.sin_port   = htons(port);
  if(inet_aton(ip, &pdata->sockaddr.sin_addr) == 0){
    _E("Failed to convert IP address");
    ret = ICOM_EINVAL;
    goto failure_inet_aton;
  }

  /* attempt connectn */
  if(connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    if(errno == ECONNREFUSED){
      //_SW("Failed to connect socket (Receiver is not yet running)");
      not_connected = 1;
    } else {
      _SE("Failed to connect socket");
      ret = (icomStatus_t)ICOM_ELINK;
      goto failure_connect;
    }
  }

  /* set up handlers */
  link->sendHandlerSecondary = (flags & ICOM_FLAG_ZERO) ? (link_sendZero) : (link_sendDefault);
  link->sendHandler = (not_connected) ? (link_connectAndSend) : (link->sendHandlerSecondary);
  link->recvHandler = link_recvNotImplemented;

  pdata->ip   = strdup(ip);
  pdata->port = port;
  link->pdata = pdata;
  link->flags = flags;
  link->type  = type;

  return ICOM_SUCCESS;


failure_connect:
failure_inet_aton:
  close(pdata->fd);
failure_socket:
  free(pdata);
  return ret;
}

icomStatus_t icom_initSocketBind(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  icomStatus_t ret;
  icomLinkSocket_t *pdata;
  uint16_t port;
  const char *p;
  char ip[sizeof("xxx.xxx.xxx.xxx")];
  int tmp;

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%15[^:]:%hu", ip, &port);
  if(r != 2){
    _E("Failed to parse communication string");
    return ICOM_EINVAL;
  }

  /* allocating memory for the private link data structure */
  pdata = (icomLinkSocket_t*)malloc(sizeof(icomLinkSocket_t));
  if(!pdata){
    _E("Failed to allocate memory");
    return ICOM_ENOMEM;
  }

  /* creating TCP socket */
  pdata->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(pdata->fd == -1){
    _SE("Failed to create socket");
    ret = (icomStatus_t)errno;
    goto failure_socket;
  }

  /* turn on socket reuse to avoid kernel holding the socket after shutdown/close */
  tmp = 1;
  ret = setsockopt(pdata->fd,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(int));
  if(ret == -1){
    _SE("Failed to set socket option");
    ret = (icomStatus_t)errno;
    goto failure_setsockopt;
  }

  /* set sender socket */
  memset(&pdata->sockaddr, 0, sizeof(struct sockaddr_in));
  pdata->sockaddr.sin_family = AF_INET;
  pdata->sockaddr.sin_port   = htons(port);
  if(strcmp(ip, "*") == 0){
    pdata->sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else if(inet_aton(ip, &pdata->sockaddr.sin_addr) == 0){
    _E("Failed to convert IP address");
    ret = (icomStatus_t)ICOM_EINVAL;
    goto failure_inet_aton;
  }

  /* bind to the IP address */
  if(bind(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to bind socket");
    ret = (icomStatus_t)errno;
    goto failure_bind;
  }

  if(listen(pdata->fd, 1) == -1){
    _SE("Failed to mark socket passive");
    ret = (icomStatus_t)errno;
    goto failure_listen;
  };

  /* set up handlers */
  link->sendHandler = link_sendNotImplemented;
  link->recvHandler = link_acceptAndRecv;

  pdata->ip         = strdup(ip);
  pdata->port       = port;
  pdata->fdAccepted = 0;
  link->pdata       = pdata;
  link->flags       = flags;
  link->type        = type;
  link->recvBufSize = 0;
  link->recvBuf     = (void*)malloc(sizeof(link));
  *(icomLink_t**)link->recvBuf = link;
  link->recvBuf     += sizeof(link);

  return ICOM_SUCCESS;


failure_listen:
failure_bind:
failure_inet_aton:
failure_setsockopt:
  close(pdata->fd);
failure_socket:
  free(pdata);
  return ret;
}

void icom_deinitSocket(icomLink_t* link){
  /* retreive private data structure */
  icomLinkSocket_t *pdata = (icomLinkSocket_t*)(link->pdata);

  if(link->type == ICOM_TYPE_SOCKET_RX && pdata->fdAccepted){
    shutdown(pdata->fdAccepted, SHUT_RDWR);
    close(pdata->fdAccepted);
  }
  shutdown(pdata->fd, SHUT_RDWR);
  close(pdata->fd);
  free(pdata->ip);

  if(link->type == ICOM_TYPE_SOCKET_RX && link->recvBuf){
    free(link->recvBuf-sizeof(link));
  }

  free(link->pdata);
}

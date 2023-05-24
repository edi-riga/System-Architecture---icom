#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "icom_macro.h"
#include "link_socket.h"
#include "notification.h"
#include "config.h"


static icomStatus_t link_nop(icomLink_t *link, void **buf, unsigned *bufSize) {
  return ICOM_SUCCESS;
}

static icomStatus_t link_error(icomLink_t *link, void **buf, unsigned *bufSize) {
  return ICOM_ERROR;
}

static icomStatus_t link_accept(icomLink_t *link, void **buf, unsigned *bufSize) {
  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;
  
  /* Connect to the */
  if (!pdata->fdAccepted) {  // TEMP
    pdata->fdAccepted = accept(pdata->fd, NULL, NULL);
    if (pdata->fdAccepted == -1) {
      _SE("Failed to accept socket");
      pdata->fdAccepted = 0;
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        _D("Timeout");
        return ICOM_TIMEOUT;
      }
      return ICOM_ERROR;
    }
  }
  return ICOM_SUCCESS;
}

static icomStatus_t link_recvHeader(icomLink_t *link, void **buf, unsigned *bufSize) {
  icomMsgHeader_t header;
  int bytesReceived;
  int ret;

  _D("Receiving at link: %p", link);

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Receive header */
  bytesReceived = 0;
  do {
    ret = recv(pdata->fdAccepted, (uint8_t*)&header+bytesReceived, sizeof(header)-bytesReceived, 0);
    if (ret == -1) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        _D("Timeout");
        return ICOM_TIMEOUT;
      }

      _SE("Receive failed (header)");
      return ICOM_ERROR;
    }

    bytesReceived += ret;
  } while ((ret != -1) && (bytesReceived < sizeof(header)));

  _D("Link @%p in header buffer @%p  received %u bytes", link, &header, ret);
  _D("Header type: %u; flags: %u; bufSize: %u", header.type, header.flags, header.bufSize);

  /* Reallocate input buffer */
  if (link->recvBufSize != header.bufSize) {
    link->recvBufSize =  header.bufSize;
    link->recvSize    =  (header.flags & ICOM_FLAG_ZERO) ? sizeof(void*) : header.bufSize;
    link->recvBuf     =  (void*)realloc(link->recvBuf-sizeof(link), sizeof(link) + link->recvBufSize);
    link->recvBuf     += sizeof(link);
    link->flags       =  header.flags;
  }
  return ICOM_SUCCESS;
}

static icomStatus_t link_recvData(icomLink_t *link, void **buf, unsigned *bufSize) {
  int bytesReceived = 0;
  int ret;

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  do {
    ret = recv(pdata->fdAccepted, (uint8_t*)(link->recvBuf)+bytesReceived, link->recvSize-bytesReceived, 0);
    if(ret == -1){
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        _D("Timeout");
        return ICOM_TIMEOUT;
      }

      _SE("Receive failed (header)");
      return ICOM_ERROR;
    }

    bytesReceived += ret;
  } while ((ret != -1) && (bytesReceived < link->recvSize));

  /* Setup output arguments */
  *buf     = (link->flags & ICOM_FLAG_ZERO) ? *(void**)link->recvBuf : link->recvBuf;
  *bufSize = (link->flags & ICOM_FLAG_ZERO) ? link->recvBufSize      : bytesReceived;

  /* Check if size of requested and sent data is equal */
  if (!(link->flags & ICOM_FLAG_ZERO) && bytesReceived != link->recvBufSize) {
    _W("Received partial data (%d bytes / %d bytes)", bytesReceived, link->recvBufSize);
    return ICOM_PARTIAL;
  }

  _D("Link @%p in buffer @%p  received %u bytes", link, link->recvBuf, *bufSize);

  return ICOM_SUCCESS;
}

static icomStatus_t link_sendHeader(icomLink_t *link, void **buf, unsigned *bufSize) {
  icomMsgHeader_t header = (icomMsgHeader_t){link->type, link->flags, *bufSize};

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  if (send(pdata->fdAccepted, &header, sizeof(header), 0) == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      _D("Send timeout");
      return ICOM_TIMEOUT;
    }
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }

  return ICOM_SUCCESS;
}

static icomStatus_t link_sendData(icomLink_t *link, void **buf, unsigned *bufSize){
  int ret;
  unsigned sendSize;

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  _D("Sending data from %p (%u bytes)", *buf, *bufSize);

  /* Send data */
  if (link->flags & ICOM_FLAG_ZERO) {
    ret = send(pdata->fdAccepted, buf, sizeof(void *), 0);
    sendSize = sizeof(void *);
  } else {
    ret = send(pdata->fdAccepted, *buf, *bufSize, 0);
    sendSize = *bufSize;
  }
  if (ret == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      _D("Send timeout");
      return ICOM_TIMEOUT;
    }

    _SE("Send failed (data)");
    return ICOM_ERROR;
  }

  /* Check if size of requested and sent data is equal */
  if (ret != sendSize) {
    return ICOM_PARTIAL;
  }

  return ICOM_SUCCESS;
}

static icomStatus_t link_connect(icomLink_t *link, void **buf, unsigned *bufSize){
  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  _D();

  /* Connect to the */
  if (!pdata->fdAccepted) {
    if (connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1) {
      _SE("Failed to connect socket");
      if (errno == ECONNREFUSED) {
        return ICOM_ECONNREFUSED;
      } else {
        return ICOM_ERROR;
      }
    }
    pdata->fdAccepted = pdata->fd;
  }

  return ICOM_SUCCESS;
}

static icomStatus_t link_sendAck(icomLink_t *link, void **buf, unsigned *bufSize){
  int ack = 1;

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  if (send(pdata->fdAccepted, &ack, sizeof(ack), 0) == -1) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      _D("Send timeout");
      return ICOM_TIMEOUT;
    }
    _SE("Send failed (ack)");
    return ICOM_ERROR;
  }

  return ICOM_SUCCESS;
}

static icomStatus_t link_recvAck(icomLink_t *link, void **buf, unsigned *bufSize) {
  int ack;
  int bytesReceived;
  int ret;

  _D("Receiving at link: %p", link);

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Receive */
  bytesReceived = 0;
  do {
    ret = recv(pdata->fdAccepted, (uint8_t*)&ack+bytesReceived, sizeof(ack)-bytesReceived, 0);
    if (ret == -1) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        _D("Timeout");
        return ICOM_TIMEOUT;
      }
      _SE("Receive failed (ack)");
      return ICOM_ERROR;
    }
    bytesReceived += ret;
  } while ((ret != -1) && (bytesReceived < sizeof(ack)));

  if (ack != 1) {
    return ICOM_ERROR;
  }
  return ICOM_SUCCESS;
}

static icomStatus_t link_sendHandler(icomLink_t *link, void *buf, unsigned bufSize){
  icomStatus_t ret;
  ret = link_connect(link, &buf, &bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link_sendHeader(link, &buf, &bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link_sendData(link, &buf, &bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link->autoRecvAck(link, buf, &bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  return ICOM_SUCCESS;
}

static icomStatus_t link_recvHandler(icomLink_t *link, void **buf, unsigned *bufSize){
  icomStatus_t ret;
  ret = link_accept(link, buf, bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link->autoSendAck(link, buf, bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link_recvHeader(link, buf, bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  ret = link_recvData(link, buf, bufSize);
  if (ret != ICOM_SUCCESS) return ret;
  return ICOM_SUCCESS;
}

static icomStatus_t link_autoSendAck(icomLink_t *link, void **buf, unsigned *bufSize){
  link->autoSendAck = link_sendAck;
  return ICOM_SUCCESS;
}

static icomStatus_t link_autoRecvAck(icomLink_t *link, void **buf, unsigned *bufSize){
  link->autoRecvAck = link_recvAck;
  return link_recvAck(link, buf, bufSize);
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

  /* set timeout (if requested) */
  if(flags & ICOM_FLAG_TIMEOUT){
    struct timeval timeout;
    timeout.tv_sec  = g_timeout_usec/1000000;
    timeout.tv_usec = g_timeout_usec%1000000;
    if( setsockopt(pdata->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
      _SW("Failed to set socket timeout option");
    }
    if( setsockopt(pdata->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0){
      _SW("Failed to set socket timeout option");
    }
  }
  
  /* set up handlers */
  link->sendHandler = link_sendHandler;
  link->recvHandler = link_error;
  link->autoSendAck = link_nop;
  link->autoRecvAck = link_nop;
  link->notifySendHandler = link_nop;
  link->notifyRecvHandler = link_nop;
  if (flags & ICOM_FLAG_AUTONOTIFY) {
    link->autoRecvAck = link_autoRecvAck;
    link->notifySendHandler = link_error;
  } else if (flags & ICOM_FLAG_NOTIFY) {
    link->notifyRecvHandler = link_recvAck;
    link->notifySendHandler = link_error;
  } else { 
    link->recvHandler = link_recvHandler;
  }

  //if (flags & ICOM_FLAG_ZERO) {
  //  printf("zero\n");
  //} else {
  //  printf("not zero\n");
  //}

  pdata->fdAccepted = 0;

  pdata->ip   = strdup(ip);
  pdata->port = port;
  link->pdata = pdata;
  link->flags = flags;
  link->type  = type;
  link->recvSize    = 0;
  link->recvBufSize = 0;
  link->recvBuf     = (void*)malloc(sizeof(link));
  *(icomLink_t**)link->recvBuf = link;
  link->recvBuf     += sizeof(link);

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

  /* set timeout (if requested) */
  if(flags & ICOM_FLAG_TIMEOUT){
    struct timeval timeout;
    timeout.tv_sec  = g_timeout_usec/1000000;
    timeout.tv_usec = g_timeout_usec%1000000;
    if( setsockopt(pdata->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
      _SW("Failed to set socket timeout option");
    }
    if( setsockopt(pdata->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0){
      _SW("Failed to set socket timeout option");
    }
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
  link->recvHandler = link_recvHandler;
  link->sendHandler = link_error;
  link->autoSendAck = link_nop;
  link->autoRecvAck = link_nop;
  link->notifySendHandler = link_nop;
  link->notifyRecvHandler = link_nop;
  if (flags & ICOM_FLAG_AUTONOTIFY) {
    link->autoSendAck = link_autoSendAck;
    link->notifyRecvHandler = link_error;
  } else if (flags & ICOM_FLAG_NOTIFY) {
    link->notifySendHandler = link_sendAck;
    link->notifyRecvHandler = link_error;
  } else {
    link->sendHandler = link_sendHandler;
  }

  pdata->ip         = strdup(ip);
  pdata->port       = port;
  pdata->fdAccepted = 0;
  link->pdata       = pdata;
  link->flags       = flags;
  link->type        = type;
  link->recvSize    = 0;
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

  if (link->type == ICOM_TYPE_SOCKET_RX && pdata->fdAccepted) {
    shutdown(pdata->fdAccepted, SHUT_RDWR);
    close(pdata->fdAccepted);
  }
  shutdown(pdata->fd, SHUT_RDWR);
  close(pdata->fd);
  free(pdata->ip);

  if (link->type == ICOM_TYPE_SOCKET_RX && link->recvBuf) {
    free(link->recvBuf-sizeof(link));
  }

  free(link->pdata);
}


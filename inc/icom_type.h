#ifndef _ICOM_TYPE_H_
#define _ICOM_TYPE_H_

typedef enum {
  ICOM_TYPE_SOCKET_CONNECT=0,
  ICOM_TYPE_SOCKET_BIND,
  ICOM_TYPE_FIFO,
  ICOM_TYPE_ZMQ_PUSH,
  ICOM_TYPE_ZMQ_PULL,
  ICOM_TYPE_ZMQ_PUB,
  ICOM_TYPE_ZMQ_SUB,
  ICOM_TYPE_ZMQ_REQ,
  ICOM_TYPE_ZMQ_REP,
  ICOM_TYPE_AUTO,
} icomType_t;


/** @brief Converts ICOM connection type ID into a string representation.
 *
 *  @param type Type of the ICOM communication connection.
 *
 *  @return String representation of the type ID.
 **/
const char* icom_typeToString(icomType_t type);


#endif

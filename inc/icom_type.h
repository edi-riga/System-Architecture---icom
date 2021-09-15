#ifndef _ICOM_TYPE_H_
#define _ICOM_TYPE_H_

typedef enum {
  ICOM_TYPE_SOCKET_TX=0,
  ICOM_TYPE_SOCKET_RX,
  ICOM_TYPE_FIFO_TX,
  ICOM_TYPE_FIFO_RX,
  ICOM_TYPE_ZMQ_PUSH,
  ICOM_TYPE_ZMQ_PULL,
  ICOM_TYPE_ZMQ_PUB,
  ICOM_TYPE_ZMQ_SUB,
  ICOM_TYPE_ZMQ_REQ,
  ICOM_TYPE_ZMQ_REP,
  ICOM_TYPE_AUTO,
  ICOM_TYPE_NONE
} icomType_t;


/** @brief Converts ICOM connection type ID into a string representation.
 *
 *  @param type Type of the ICOM communication connection.
 *
 *  @return String representation of the type ID.
 **/
const char* icom_typeToString(icomType_t type);

/** @brief Converts ICOM connection type string representation into ID
 *
 *  @param typeString String representation of the type ID.
 *
 *  @return Type of the ICOM communication connection. On failure
 *         ICOM_TYPE_NONE is returned
 **/
icomType_t icom_stringToType(const char *typeString);

#endif

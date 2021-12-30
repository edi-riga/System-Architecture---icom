#ifndef _CONFIG_H_
#define _CONFIG_H_

/* The maximum string length, which can be read using scan for the communication
 * type identifier. */
#ifndef MAX_TYPE_STRING_LENGTH
  #define MAX_TYPE_STRING_LENGTH 63
#endif

/* Delimeter character for settting ICOM communication strings, the default
 * format is: "<type>|<flags>|<communication-string>"
 * Examples:
 *   - "socket_tx|default|127.0.0.1:8888"
 *   - "socket_rx|default|*:8888"*/
#ifndef ICOM_DELIMITER
  #define ICOM_DELIMITER  '|'
#endif

#endif

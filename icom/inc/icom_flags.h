#ifndef _ICOM_FLAGS_H_
#define _ICOM_FLAGS_H_

#include <stdint.h>

/* icom flags data type */
typedef uint32_t icomFlags_t;

#define ICOM_FLAG_DEFAULT    (0)
#define ICOM_FLAG_ZERO       (1<<0)
#define ICOM_FLAG_PROT       (1<<1)
#define ICOM_FLAG_TIMEOUT    (1<<2)
#define ICOM_FLAG_NOTIFY     (1<<3)
#define ICOM_FLAG_AUTONOTIFY (1<<4)
#define ICOM_FLAG_MAX_VALID  ICOM_FLAG_AUTONOTIFY
#define ICOM_FLAG_ZERO_PROT  ((1<<0)+(1<<1))
#define ICOM_FLAG_INVALID    (1<<31)


/** @brief Converts ICOM connection flags into a string representation.
 *
 *  @param flags Flags for the ICOM communication connection.
 *
 *  @return String representation of the flags.
 **/
const char* icom_flagsToString(icomFlags_t flags);


/** @brief Converts ICOM connection flag string representation into flag type
 *         representation.
 *
 *  @param flagString String representation of the communication flags.
 *
 *  @return Flags of the ICOM communication connection. On failure
 *         contains ICOM_FLAG_INVALID bit.
 **/
icomFlags_t icom_stringToFlags(const char *flagString);


#endif

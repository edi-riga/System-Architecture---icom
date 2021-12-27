#ifndef _ICOM_FLAGS_H_
#define _ICOM_FLAGS_H_

#include <stdint.h>

/* icom flags data type */
typedef uint32_t icomFlags_t;


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

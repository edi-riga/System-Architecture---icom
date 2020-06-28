#ifndef _STRING_PARSER_H_
#define _STRING_PARSER_H_


/*@ Takes an ZMQ communication string as input with the possibility of 
 *  specifying ranges, thus the necessity for multiple sockets and initializea
 *  a C string array (must be deinitialized)
 *
 *  @param strArray Address where to save pointer to the initialized 
 *         communication C-string array
 *  @param strCount Address to the unsigned variable where to save the number
 *         of found communication strings
 *  @param ptrStart The input C-string, which shall be parsed
 *
 *  @return Returns '0' on success, '-1' otherwise */
int  parser_initStrArray(char*** strArray, unsigned* strCount, char* ptrStart);


/*@ Deallocates all memory allocated previously with parser_initStrArray 
 *  function.
 *
 *  @param strArray Previoussly allocated communication C-string array
 *  @param strCount Number of previously found communication strings
 */
void parser_deinitStrArray(char **strArray, unsigned strCount);

#endif

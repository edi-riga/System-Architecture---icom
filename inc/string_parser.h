#ifndef _STRING_PARSER_H_
#define _STRING_PARSER_H_

/*@ Takes a communication string as input with the possibility of specifying
 *  multiple ranges. This string is split into multiple separate strings and
 *  saved into strArray pointer array. If supplied ptrStart is NULL, the 
 *  function still sets strCount to 0, but the function will fail with -1.
 *
 *  @param strArray pointer to array of pointers where to save generated
 *         communication C-string array
 *  @param strCount Address to the unsigned variable where to save the number
 *         of found communication strings
 *  @param ptrStart The input C-string, which shall be parsed
 *
 *  @return Returns '0' on success, '-1' otherwise */
int  parser_initStrArray(char*** strArray, unsigned* strCount, const char* ptrStart);


/*@ Deallocates all memory allocated previously with parser_initStrArray 
 *  function.
 *
 *  @param strArray Previoussly allocated communication C-string array
 *  @param strCount Number of previously found communication strings
 */
void parser_deinitStrArray(char **strArray, unsigned strCount);

#endif

/* @author: Daniels Jānis Justs */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_parser.h"
#include "notification.h"

#ifndef _GNU_SOURCE
static inline char* strchrnul(const char *ptr, char delimiter){
  while(*ptr != delimiter && *ptr != '\0') ptr++;
  return (char*)ptr;
}
#endif

unsigned parser_getCount(const char *ptrStart){
  char *ptrStop;
  int   strCount = 0;
  int   idFrom, idTo, ret;

  do{
    // parse entry
    ptrStop = strchrnul(ptrStart, ',');
    char *tmp = strndup(ptrStart, ptrStop - ptrStart);
    ret = sscanf(tmp, "%*[^[][%d-%d]", &idFrom, &idTo);
    free(tmp);
     
    // update count 
    if(ret == 2)
      strCount += idTo - idFrom;
    strCount++;

    // set up for next iteration
    ptrStart = ptrStop + 1;
  }while(*ptrStop != '\0');
  
  return strCount;
}

int parser_initStrArray(char ***strArray, unsigned *strCount, const char *ptrStart){
  char *ptrStop, *idStr;
  int   idFrom, idTo, ret;
  unsigned strCurrent = 0;

  // check if no string has been passed
  if(ptrStart == NULL){
    *strArray = NULL;
    *strCount = 0;
    return -1;
  }

  // allocate memories
  *strCount = parser_getCount(ptrStart);
  *strArray = (char**)malloc((*strCount)*sizeof(char*));
  if(*strArray == NULL){
    _E("Failed to allocate memory");
    return -1;
  }

  do{
    // retreive candidate string
    ptrStop = strchrnul(ptrStart, ',');
    char *candidate = strndup(ptrStart, ptrStop - ptrStart);
    if(!candidate){
      _E("Failed to allocate memory");
      free(strArray);
      return -1;
    }

    // parse the candidate string
    ret = sscanf(candidate, "%m[^[][%d-%d]", &idStr, &idFrom, &idTo);
    if(ret == EOF){
      _E("Failed to parse string: %s", candidate);
      free(candidate);
      free(strArray);
      return -1;
    }

    // case with a simple string: "string"
    if(ret == 1){
      (*strArray)[strCurrent] = candidate;
      strCurrent++;
      free(idStr);
      ptrStart = ptrStop + 1;
      continue;
    }

    // special case: "string[0]", basically treat as "string[0-0]"
    if(ret == 2){
      idTo = idFrom;
    }

    // case with advanced string: "string[0-1]"
    for(int i = idFrom; i <= idTo; i++){
      unsigned size = snprintf(NULL, 0, "%s%d", idStr, i) + 1;
      (*strArray)[strCurrent] = (char*)malloc(size*sizeof(char));
      // TODO: error checking
      sprintf((*strArray)[strCurrent], "%s%d", idStr, i);
      strCurrent++;
    }

    free(candidate);
    free(idStr);

    // set up for next iteration
    ptrStart = ptrStop + 1;
  } while(*ptrStop != '\0');

  return 0;
}

void parser_deinitStrArray(char **strArray, unsigned strCount){
  for(int i=0; i<strCount; i++)
    free((strArray)[i]);
  free(strArray);
}

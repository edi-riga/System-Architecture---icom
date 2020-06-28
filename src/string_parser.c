#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_parser.h"

#ifndef _GNU_SOURCE
static inline char* strchrnul(char *ptr, char delimiter){
    while(*ptr != delimiter && *ptr != '\0') ptr++;
    return ptr;
}
#endif

unsigned parser_getCount(char *ptrStart){
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

int parser_initStrArray(char ***strArray, unsigned *strCount, char *ptrStart){
    char *ptrStop, *idStr;
    int   idFrom, idTo, ret;
    unsigned strCurrent = 0;

    // allocate memories
    *strCount = parser_getCount(ptrStart);
    *strArray = (char**)malloc((*strCount)*sizeof(char*));

    do{
        // parse entry
        ptrStop = strchrnul(ptrStart, ',');
        char *candidate = strndup(ptrStart, ptrStop - ptrStart);
        ret = sscanf(candidate, "%m[^[][%d-%d]", &idStr, &idFrom, &idTo);
       
        // update count 
        if(ret != 3){
            (*strArray)[strCurrent] = candidate;
            strCurrent++;
        }else{
            for(int i = idFrom; i <= idTo; i++){
                unsigned size = snprintf(NULL, 0, "%s%d", idStr, i) + 1;
                (*strArray)[strCurrent] = (char*)malloc(size*sizeof(char));
                sprintf((*strArray)[strCurrent], "%s%d", idStr, i);
                strCurrent++;
            }
            free(candidate);
        }
        free(idStr);

        // set up for next iteration
        ptrStart = ptrStop + 1;
    }while(*ptrStop != '\0');

    return 0;
}

void parser_deinitStrArray(char ***strArray, unsigned strCount){
    for(int i=0; i<strCount; i++)
        free((*strArray)[i]);
    free(*strArray);
}

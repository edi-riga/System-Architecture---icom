#ifndef _MACRO_H_
#define _MACRO_H_

/* C macro (sad) argument count retreival logic (max 5) */
#define ARGUMENT_SELECT(_1, _2, _3, _4, _5, N, ...)  N
#define ARGUMENT_INDEXES  5, 4, 3, 2, 1, 0
#define ARGUMENT_COUNT_(...) ARGUMENT_SELECT(__VA_ARGS__)
#define ARGUMENT_COUNT(...)  ARGUMENT_COUNT_(__VA_ARGS__, ARGUMENT_INDEXES())

/* C macro (sad) concatenation logic */
#ifndef CONCATENATE
  #define CONCATENATE(left, right) ICOM_CONCATENATE1(left, right)
  #define ICOM_CONCATENATE1(left, right) ICOM_CONCATENATE2(left, right)
  #define ICOM_CONCATENATE2(left, right) left##right
#endif

/* C macro to stringify */
#define _STR(s)  #s
#define STR(s)  _STR(s)


#endif

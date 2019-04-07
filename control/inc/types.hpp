#ifndef __TYPES_H__
#define __TYPES_H__

typedef char                char_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

typedef float               float32_t;
typedef double              float64_t;

typedef enum { RET_UNKNOWN = 0, RET_FAIL, RET_SUCCESS } return_t;

#endif

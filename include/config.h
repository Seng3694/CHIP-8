#ifndef C8_CONFIG_H
#define C8_CONFIG_H

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

#ifdef _DEBUG
#include <stdio.h>
#define LOG(...) printf(__VA_ARGS__);
#else
#define LOG(...) 
#endif

#endif
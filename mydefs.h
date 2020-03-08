#pragma once
#include <stddef.h>
#include <sys/types.h>

typedef u_int8_t ui08;
typedef u_int16_t ui16;
typedef u_int32_t ui32;
typedef u_int64_t ui64;

typedef int8_t si08;
typedef int16_t si16;
typedef int32_t si32;
typedef int64_t si64;

typedef off_t tick_t;
typedef off_t noteid_t;

#define TICK_T_MAX __INT64_MAX__
////////////////////////////////////
//~ NOTE: Platform non-specific code

#ifndef MASTER_H
#define MASTER_H

#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRECATE

// NOTE: Turn off warnings
// 4100 - unreferenced formal parameter
// 4201 - nameless struct/union
// 4189 - local variable is initialized but not referenced
// 4211 - nonstandard extension used
// 4212 - nonstandard extension used: function declaration used ellipsis
// 4255 - no function prototype given
// 4464 - realative include path contains ".."
// 4702 - unreachable code
// 4710 - function not inlined
// 4711 - function being inlined
// 4820 - structure padding
// 5045 - spectre/meltdown CPU vulnerability
#pragma warning(disable: 4100; disable: 4189; disable: 4201; disable: 4211; disable: 4212; disable: 4255; disable: 4464; disable: 4702; disable: 4710; disable: 4711; disable: 4820; disable: 5045)

// NOTE: C Standard Library
#include <math.h>
#include <stdint.h> // types
#if DEVELOPER
    #include <stdio.h> // for vsprintf_s, printf 
    #include <stdarg.h> // for the variable argument list
#endif

////////////////////////////////////
//~ NOTE: Types

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t sizeT; // size_t is 64 bits on 64bit builds and 32-bit on 32bit builds


////////////////////////////////////
//~ NOTE: Macros

#define global static
#define internal static
#define persist static

#define PI 3.14159265358979323846f
#define PI32 3.14159265359f
#define TWO_PI 6.28318530717958647693f
#define HALF_PI PI/2.f

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)
#define arrayCount(a) (sizeof(a) / sizeof(a[0]))
#define swap(x, y)\
        x = x ^ y; \
        y = x ^ y; \
        x = x ^ y;

#if 0
#define swap(x, y)\
    f32 temp = x; \
    x = y; \
    y = temp;
#endif

#define minValue(a, b) ((a < b) ? (a) : (b))
#define maxValue(a, b) ((a > b) ? (a) : (b))

inline void
internalClearMemory(void *memory, sizeT size)
{
    u8 *byte = (u8 *)memory;
    while (size--) {
        *byte++ = 0;
    }
}

#define clearArray(a) internalClearMemory(a, sizeof(a))
#define clearStruct(a) internalClearMemory(&(a), sizeof(a))

#ifndef __cplusplus
    #define false 0
    #define true 1
#endif

// for floating point when built with NOCRT
#ifdef NOCRT
    #define _NO_CRT_STDIO_INLINE
    int _fltused;
#endif

#if DEVELOPER
    #define assert(expression) if(!(expression)) { *(volatile int *)0 = 0; }
    #define invalidCodePath assert(!"Invalid code path!")
    #define invalidDefaultCase defult: { assert(!"Invalid default case!"); }
    #define debug(msg, ...) fprintf(stderr, "Debug stderr -> %s:%d: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #define debugPrint(msg, ...) printf(msg "\n", ##__VA_ARGS__)
    #define debugPrintVariable(T, var) debugPrint(#var" = %"#T, var);
#else
    #define assert(x)
    #define invalidCodePath assert(x)
    #define invalidDefaultCase
    #define debug(msg, ...)
    #define debugPrint(x, ... )
    #define debugPrintVariable(T, var)
#endif // DEVELOPER

#include <malloc.h>
// Sean Barrets stretchy buffer
// https://github.com/nothings/
#define stbArrayPush stb_sb_push
#define stbArrayCount stb_sb_count
#define stbArrayFree stb_sb_free
// initialize with n number of elements
#define arrayInit stb_sb_add

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (void *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))

static void *stb__sbgrowf(void *arr, int increment, int itemsize)
{
    int dbl_cur = arr ? 2 * stb__sbm(arr) : 0;
    int min_needed = stb_sb_count(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = (int *)realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    }
    else {
        return (void *)(2 * sizeof(int)); // try to force a NULL pointer exception later
    }
}

////////////////////////////////
//~ NOTE: Vectors

typedef union
{
    struct
    {
        i32 x, y;
    };
    
    struct
    {
        i32 width, height;
    };
    
    i32 e[2];
} v2i;

typedef union
{
    struct
    {
        f32 x, y;
    };
    
    struct
    {
        f32 width, height;
    };
    
    f32 e[2];
} v2;

typedef union
{
    struct
    {
        f32 x, y, z;
    };
    
    struct
    {
        f32 r, g, b;
    };
    
    // swizzle
    struct
    {
        v2 xy;
        f32 _ignored0;
    };
    
    struct
    {
        f32 _ignored1;
        v2 yz;
    };
    
    f32 e[3];
} v3;

typedef union
{
    struct
    {
        f32 x, y;
        union
        {
            struct
            {
                f32 z, w;
                
            };
            struct
            {
                f32 width, height;
            };
        };
    };
    
    struct
    {
        f32 r, g, b, a;
    };
    
    f32 e[4];
} v4;

#define v2(...) (v2){ __VA_ARGS__ }
#define v2i(...) (v2i){ __VA_ARGS__ }
#define v3(...) (v3){ __VA_ARGS__ }
#define v4(...) (v4){ __VA_ARGS__ }

#if 0
#define v2i(x, y) v2iInit(x, y)
v2i v2iInit(i32 x, i32 y)
{
    v2i result = { x, y };
    return result;
}

#define v2(x, y) v2Init(x, y)
v2 v2Init(f32 x, f32 y)
{
    v2 result = { x, y };
    return result;
}

#define v3(x, y, z) v3Init(x, y, z)
v3 v3Init(f32 x, f32 y, f32 z)
{
    v3 result = { x, y, z };
    return result;
}

#define v4(x, y, z, w) v4Init(x, y, z, w)
v4 v4Init(f32 x, f32 y, f32 z, f32 w)
{
    v4 result = { x, y, z, w };
    return result;
}
#endif


////////////////////////////////
//~ NOTE: Scalars

#include <math.h>

inline i32
i32Truncate(f32 value)
{
    i32 result = (i32)value;
    return result;
}
// truncate 64bits to 32bits and assert truncation
inline u32 u32SafeTruncate(u64 value)
{
	assert(value <= 0xffffffff);
	u32 result = (u32)value;
	return result;			
}

// floor to the nearest integer
// truncates towards negative instead of zero
inline i32
i32Floor(f32 value)
{
    //i32 result = (i32)floorf(value);
    i32 result;
    if (value > 0)
    {
        result = (i32)value;
    }
    else
    {
        result = (i32)value - 1;
    }
    return result;
}

// ceil to the nearest integer
inline i32
i32Ceil(f32 value)
{
   // i32 result = (i32)ceilf(value);
    i32 result;
    if (value > 0)
    {
        result =  (i32)value + 1;
    }
    else
    {
        result = (i32)value;
    }
    return result;
}

// round down a float value to nearest integer
inline i32
i32Round(f32 value)
{
    i32 result = (i32)(value + 0.5f);
    //i32 result = (i32)roundf(value); // not working with nocrt
    return result;
}

// round down a float value to nearest unsigned integer
inline u32
u32Round(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    //u32 result = (u32)roundf(value); // not working with nocrt
    return result;
}

inline f32
f32Floor(f32 value)
{
    f32 result;
    if (value > 0)
        result = (f32)((i32)value);
    else
        result =  (f32)((i32)value) - 1;
    
    return result;
}

inline f32
f32Ceil(f32 value)
{
    f32 result;
    if (value > 0)
        result =  (f32)((i32)value) + 1;
    else
        result =  (f32)((i32)value);
    
    return result;
}


////////////////////////////////
//~ NOTE: Trigonometry

// sinf32(), cosf32() takes radians as input and returns a number between -1 and 1
inline f32 sinf32(f32 angle)
{
    f32 result = sinf(angle);
    return result;
}

inline f32 cosf32(f32 angle)
{
    f32 result = cosf(angle);
    return result;
}



//
// Random numbers
//
#include <stdlib.h>
// pseudo-random uniform distribution of numbers
// set the random seed to a constant value to return the same pseudo random numbers every time
inline void randomSeed(u32 value)
{
	srand(value);
}

// returns random integer between min and max
// ex:  random(-10,20) -> will produce -10 to, and including, 20.
inline i32 random(i32 min, i32 max)
{
	max += 1;
	min -= 1;
	i32 result = (i32)((rand() / (f32)(RAND_MAX + 1) * (max - min) + min));
	return result;
}

// returns random integer between 0 and max
inline i32 randomZeroToMax(i32 max)
{
	// i32 min = -1;
	max += 1;
	return (i32)(rand() / (f32)(RAND_MAX + 1) * (max));
}

// returns a random float between 0 and 1
inline f32 randomZeroToOnef()
{
    f32 result =((f32)(rand() / (f32)(RAND_MAX)));
	//random float between -1 and 1
	//return (((f32)rand() / (RAND_MAX)) * 2 - 1.0f);
	return result;
}

// returns random float between min and max
inline f32 randomf(f32 min, f32 max)
{
	return min + randomZeroToOnef() * (max - min);
}
#endif // MASTER_H
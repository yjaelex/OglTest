#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _WIN32
#include <windows.h>
#else
#ifdef LINUX
#include <GL/glx.h>
#endif
#endif

#if !defined(_MSC_VER) && !defined(__GNUC__)
#error "Unknown compiler, need to explicitly define data types!!!"
#endif // !MSC_VER && !__GNUC__

#if defined(_WIN32)
#include <stddef.h>
#elif defined(LINUX)
#include <stdint.h>
#include <stdbool.h>
#else
#error "Unsupported OS."
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1700)
#define COMPILE_TIME_ASSERT(pred) static_assert(pred, #pred)
#else
#define COMPILE_TIME_ASSERT(pred) switch(0) { case 0: case (pred): break; }
#endif

#if !defined(UINT_MAX)                  ///< limits.h on linux
#define UINT_MAX 0xffffffff             ///< maximum unsigned int value 
#endif 

#ifndef UNREFERENCED_PARAMETER
/// use this macro on unreferenced variables to avoid compiler warnings.
#define UNREFERENCED_PARAMETER(P)          \
    { \
        (P) = (P); \
    } \
#endif

#if defined(COMP_VC)
/// 64-bit signed integer
typedef signed __int64 int64, *pint64;

/// 64-bit unsigned integer
typedef unsigned __int64 uint64, *puint64;
#endif // COMP_VC


#if defined(COMP_GCC)
/// 64-bit signed integer
typedef signed long long int64, *pint64;;

/// 64-bit unsigned integer
typedef unsigned long long uint64, *puint64;
#endif  // COMP_GCC


/// 8-bit character
typedef unsigned char char8, *pchar8;

/// 8-bit signed integer
typedef signed char int8, *pint8;

/// 16-bit signed integer
typedef signed short int16, *pint16;

/// 32-bit signed integer
typedef signed int int32, *pint32;

/// 8-bit unsigned integer
typedef unsigned char uint8, *puint8;

/// 16-bit unsigned integer
typedef unsigned short uint16, *puint16;

/// 32-bit unsigned integer
typedef unsigned int uint32, *puint32;

/// 32-bit floating point
typedef float float32, *pfloat32;

/// 64-bit floating point
typedef double float64, *pfloat64;

/// pointer to void
typedef void *pvoid;

/// unsigned integer, the same size as a pointer
typedef uintptr_t uintp;

/// signed integer, the same size as a pointer
typedef intptr_t intp;

using namespace std;

#endif


#define ArraySize(_arr) ( sizeof((_arr)) / sizeof((_arr)[0]) )

void debug(const char* fmt, ...);
void log(const char* fmt, ...);
void warn(const char* fmt, ...);
void error(const char* fmt, ...);

bool CheckError(const char* Title);

#endif

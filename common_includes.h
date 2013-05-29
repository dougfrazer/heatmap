#ifndef __COMMON_INCLUDES_H__
#define __COMMON_INCLUDES_H__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "windows.h"

typedef unsigned int uint;
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

#define null            0
#define error(...)      assert(false)
#define countof(x)      sizeof(x)/sizeof(*x)
#define bitcountof(x)   sizeof(x)*8
#define alignup(x, v)   ( x + v - 1 & ~(v - 1) )
#define clamp(x, min, max) ( x > max ? max : x < min ? min : x )
inline void pointer_make_relative(void** x) { *x = (void*)((intptr_t)*x + 1 - (intptr_t)x); }
inline void pointer_make_absolute(void** x) { *x = (void*)((intptr_t)*x - 1 + (intptr_t)x); }
__forceinline void* Malloc( size_t size, const char* File = __FILE__, int Line = __LINE__ ) {
	char string[256];
	sprintf_s(string, 256, "[%s:%d] Allocation: %d\n", File, Line, size);
	OutputDebugString(string);
	return malloc(size);
}
__forceinline void Free( void* ptr, const char* File = __FILE__, int Line = __LINE__ ) {
	char string[256];
	sprintf_s(string, 256, "[%s:%d] Free: %p\n", File, Line, ptr);
	OutputDebugString(string);
	free(ptr);
}

#endif
#ifndef __COMMON_INCLUDES_H__
#define __COMMON_INCLUDES_H__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "windows.h"
#include <strsafe.h>

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
#define LinearInterpolate(x, x0, x1, y0, y1) ( y0+ ( (float)(y1-y0) * ( (float)(x-x0)/(float)(x1-x0) ) ) )

inline void pointer_make_relative(void** x) { *x = (void*)((intptr_t)*x + 1 - (intptr_t)x); }
inline void pointer_make_absolute(void** x) { *x = (void*)((intptr_t)*x - 1 + (intptr_t)x); }

#define Malloc( size ) MallocInternal( size, __FILE__, __LINE__ )
inline void* MallocInternal( size_t size, const char* File = __FILE__, int Line = __LINE__ ) {
	char string[256];
	sprintf_s(string, 256, "[%s:%d] Allocation: %d\n", File, Line, size);
	OutputDebugString(string);
	return malloc(size);
}

#define Free( ptr ) FreeInternal( ptr, __FILE__, __LINE__ )
inline void FreeInternal( void* ptr, const char* File = __FILE__, int Line = __LINE__ ) {
	char string[256];
	sprintf_s(string, 256, "[%s:%d] Free: %p\n", File, Line, ptr);
	OutputDebugString(string);
	free(ptr);
}

inline void PrintError(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}


#endif
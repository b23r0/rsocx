#pragma once

#include <stdio.h>

#ifdef LINUX
	
	#define _T(str) str
	#define WINAPI
	#define TRUE true
	#define FALSE false

	typedef char* LPSTR;
	typedef bool BOOL;
	typedef void* LPVOID;
	typedef const char* LPCSTR;
	typedef const char* LPCTSTR;
	typedef unsigned long DWORD;
	typedef void* LPVOID;
	typedef long LONG;
	typedef unsigned int UINT;
	typedef unsigned short WORD;
	typedef unsigned char BYTE;
	typedef unsigned long DWORD_PTR;
	typedef int SOCKET;

	typedef void* (*LPTHREAD_START_ROUTINE)(void*);


	#define WSAGetLastError() 1

	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <time.h>
	#include <stdlib.h>
	#include <wait.h>

#else

	#include <tchar.h>
	#include <process.h>
	#include <winsock2.h>
	#pragma comment(lib,"ws2_32.lib")

#endif


#include "utils/AutoCleanup.h"
#include "utils/ods.h"
#include "utils/tstring.h"
#include "common/CommonDefines.h"

#pragma once

#include <stdarg.h>

#define OUTPUT_LOG
#define ODS_OUTPUT_STD

#define ODS_LOG_MAXLENGTH 1024

#ifdef LINUX
	#define GetLastError() errno
#endif

#ifdef OUTPUT_LOG
#	define errorLogE ErrorODSE
#	define errorLog ErrorODS
#	define infoLogE InfoODSE
#	define infoLog InfoODS
#	define debugLogE DebugODSE
#	define debugLog DebugODS
#else
#	define errorLogE 
#	define errorLog 
#	define infoLogE 
#	define infoLog 
#	define debugLogE 
#	define debugLog 
#endif


#ifdef _DEBUG
#define ODS_LEVEL ODSLEVEL_DEBUG
#else
#define ODS_LEVEL ODSLEVEL_INFO
#endif

typedef enum
{
	ODSLEVEL_DEBUG = 0,
	ODSLEVEL_INFO,
	ODSLEVEL_ERROR,
} LOG_LEVEL;

void WriteDebugLog(DWORD dwLastError, LPCSTR file, int codeLine, LOG_LEVEL level, LPCTSTR content, ...);

#define DebugODS(fmt, ...) WriteDebugLog(0, __FILE__, __LINE__, ODSLEVEL_DEBUG, fmt, ##__VA_ARGS__)

#define DebugODSE(fmt, ...) WriteDebugLog(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_DEBUG, fmt, ##__VA_ARGS__)

#define InfoODS(fmt, ...) WriteDebugLog(0, __FILE__, __LINE__, ODSLEVEL_INFO, fmt, ##__VA_ARGS__)

#define InfoODSE(fmt, ...) WriteDebugLog(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_INFO, fmt, ##__VA_ARGS__)

#define ErrorODS(fmt, ...) WriteDebugLog(0, __FILE__, __LINE__, ODSLEVEL_ERROR, fmt, ##__VA_ARGS__)

#define ErrorODSE(fmt, ...) WriteDebugLog(::GetLastError(), __FILE__, __LINE__, ODSLEVEL_ERROR, fmt, ##__VA_ARGS__)

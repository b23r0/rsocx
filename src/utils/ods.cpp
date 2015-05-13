#include "stdafx.h"
#include <algorithm>
#include <stdlib.h>
#include "tstring.h"
#include "ods.h"
#include "AutoCleanup.h"

static CriticalSection g_fileSection;

#ifdef LINUX

typedef char TCHAR;

#define _TRUNCATE ((size_t)-1)
#define GetCurrentThreadId pthread_self
#define s2ws
#define _stprintf_s sprintf
#define _tcslen strlen
#define _tcscat_s strcat
#define _vsntprintf_s vsnprintf

#endif

#ifndef LINUX

void OutputFile(LPCTSTR content)
{
	static BOOL bOpened = FALSE;
	g_fileSection.Enter();
	{
		do 
		{
			HANDLE hFile = CreateFile(_T("C:\\ods.log"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
			if (hFile != INVALID_HANDLE_VALUE)
				::SetFilePointer(hFile, 0, NULL, FILE_END);
			else
				break;

			std::string ansiContent = t2a(content);
			ansiContent += "\r\n";
			DWORD dwWritten = 0 ;
			WriteFile(hFile,(LPBYTE)ansiContent.c_str(), ansiContent.size(),&dwWritten,0);

			CloseHandle(hFile);
		} while (FALSE);
	}
	g_fileSection.Leave();
}
#endif


void WriteDebugLog(DWORD dwLastError, LPCSTR file, int codeLine, LOG_LEVEL level, LPCTSTR content, ...)
{
	if(level < ODS_LEVEL)
		return;

	//格式化日志
	TCHAR logContent[ODS_LOG_MAXLENGTH + 1] = {0};

	DWORD dwThreadID = ::GetCurrentThreadId();
	std::string ansiCodefile = file;
	std::string::size_type pos = ansiCodefile.find_last_of('\\');
	if (pos != std::string::npos && pos + 1 < ansiCodefile.size()) ansiCodefile = ansiCodefile.substr(pos + 1);
	tstring codeFile = s2ws(ansiCodefile);

	tstring strLevel;
	switch (level)
	{
	case ODSLEVEL_DEBUG:
		strLevel = _T("[D]");
		break;
	case ODSLEVEL_INFO:
		strLevel = _T("[+]");
		break;
	case ODSLEVEL_ERROR:
		strLevel = _T("[-]");
		break;
	default:
		strLevel = _T("[?]");
		break;
	}
#if _DEBUG
	int iWritten = _stprintf_s(logContent, _T("%s [%s:%d] %u "), strLevel.c_str(), codeFile.c_str(), codeLine, dwThreadID);
#else
	int iWritten = _stprintf_s(logContent, _T("%s "), strLevel.c_str());
#endif

	va_list ap;
	va_start(ap, content);
#ifdef LINUX
	vsnprintf(logContent + iWritten, ODS_LOG_MAXLENGTH - iWritten, content, ap);
#else
	_vsntprintf_s(logContent + iWritten, ODS_LOG_MAXLENGTH - iWritten, _TRUNCATE, content, ap);
#endif
	va_end(ap);

	if (dwLastError != 0)
	{
		TCHAR lastError[16] = {0};
		_stprintf_s(lastError, _T(" E%d"), dwLastError);
		
		size_t len = _tcslen(logContent);
		if (len + _tcslen(lastError) < ODS_LOG_MAXLENGTH)
		{
			_tcscat_s(logContent, lastError);
		}
	}
#ifdef LINUX
	std::string log = logContent;
#else
	std::wstring log = logContent;
#endif
	std::transform(log.begin(),log.end(),log.begin(),toupper);

#ifdef ODS_OUTPUT_STD
	printf("%s\n", t2a(log.c_str()));
#endif

#ifndef LINUX
#ifdef ODS_OUTPUT_FILE
 	OutputFile(logContent);
#endif
	OutputDebugString(logContent);
#endif
}
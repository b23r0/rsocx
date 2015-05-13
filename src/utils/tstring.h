#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <set>

#ifdef LINUX
	
	typedef char* LPCWSTR;

	#define tstring std::string
	#define a2t(str) (str)
	#define t2a(str) (str)

#else
	
	#include <windows.h>
	#ifdef _UNICODE
		#define tstring std::wstring
		#define tostringstream std::wostringstream
		#define a2t(str) (ansi2wchar(str, (int)strlen(str)).c_str())
		#define t2a(str) (wchar2ansi(str).c_str())
	#else
		#define tstring std::string
		#define tostringstream std::ostringstream
		#define a2t(str) (str)
		#define t2a(str) (str)
	#endif

#endif


std::string wchar2ansi(LPCWSTR pwszSrc);

std::string ws2s(std::wstring& inputws);

std::wstring ansi2wchar(LPCSTR pszSrc, int nLen);

std::wstring s2ws(const std::string& s);

void makeUpper(tstring& str);

tstring& makeLower(tstring& str);

void trim(tstring& str, char ch = ' ');

std::string WStringToUTF8(const wchar_t* lpwcszWString);

std::wstring UTF8ToWString(const char* lpcszString);
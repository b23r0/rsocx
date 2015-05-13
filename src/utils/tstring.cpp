#include "stdafx.h"
#include <algorithm>
#include "tstring.h"

#ifndef LINUX

std::string wchar2ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen<= 0) return std::string("");
	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");
	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;
	std::string strTemp(pszDst);
	delete [] pszDst;
	return strTemp;
}

std::string ws2s(std::wstring& inputws)
{ 
	return wchar2ansi(inputws.c_str()); 
}

std::wstring ansi2wchar(LPCSTR pszSrc, int nLen)
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	if(nSize <= 0) return std::wstring(L"");
	WCHAR *pwszDst = new WCHAR[nSize+1];
	if( NULL == pwszDst) return std::wstring(L"");

	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;
	if( pwszDst[0] == 0xFEFF) // skip Oxfeff
	{
		for(int i = 0; i < nSize; i ++)
		{
			pwszDst[i] = pwszDst[i+1]; 
		}
	}
	std::wstring wcharString(pwszDst);
	delete[] pwszDst;

	return wcharString;
}

std::wstring s2ws(const std::string& s)
{ 
	return ansi2wchar(s.c_str(), (int)s.size());
}

void makeUpper(tstring& str)
{
	transform(str.begin(), str.end(), str.begin(), toupper);
}

tstring& makeLower(tstring& str)
{
	transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

void trim(tstring& str, char ch)
{
	tstring::size_type pos = str.find_first_not_of(ch);
	if (pos == tstring::npos)
	{
		str = _T("");
		return;
	}

	tstring::size_type pos2 = str.find_last_not_of(ch);
	if (pos2 != tstring::npos)
	{
		str = str.substr(pos, pos2 - pos + 1);
	}
	else
	{
		str = str.substr(pos);
	}
}

std::wstring UTF8ToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode;
	pUnicode = new wchar_t[unicodeLen + 1];
	memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wstrReturn(pUnicode);
	delete [] pUnicode;
	return wstrReturn;
}

std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete [] pElementText;
	return strReturn;
}

#endif
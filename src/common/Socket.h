#pragma once

#include "../stdafx.h"

#ifdef LINUX

	typedef int SOCKET;
	#define SOCKET_ERROR -1

	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include "th3rd/dns.h"

#else
	typedef int socklen_t;
	#include <winsock2.h>
	#include <WS2tcpip.h>
	#pragma comment(lib,"ws2_32.lib")

#endif

#include <map>
#include <string>
typedef std::map<std::string,in_addr> DNS_CACHE;
static CriticalSection g_csCache;

static DNS_CACHE g_cache;

#define CONNECTNUM 20

namespace Socket
{
	static SOCKET Create(BOOL IsTcp = TRUE)
	{
		return socket(AF_INET,IsTcp ? SOCK_STREAM : SOCK_DGRAM,0);
	}

	static BOOL Bind(SOCKET s,int port,sockaddr_in& addr)
	{
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		s = bind(s,(sockaddr*)&addr,sizeof(addr));

		return s != SOCKET_ERROR;
	}

	static SOCKET Accept(SOCKET s,sockaddr* out_addr)
	{
		socklen_t nSize = sizeof(sockaddr);
		return accept(s,out_addr,&nSize);
	}

	static BOOL Connect(SOCKET s, LPCSTR ip,int port )
	{
		int size = 0;

		sockaddr_in r_addr;

		memset(&r_addr, 0, sizeof(sockaddr_in));

		r_addr.sin_port=htons(port);
		r_addr.sin_family=AF_INET;
		r_addr.sin_addr.s_addr = inet_addr(ip);

		s = connect(s,(struct sockaddr *)&r_addr,sizeof(sockaddr));

		return s == 0;
	}

	static BOOL Connect(SOCKET s, sockaddr_in& r_addr )
	{
		s = connect(s,(struct sockaddr *)&r_addr,sizeof(sockaddr));

		return s == 0;
	}

	static BOOL Listen(int s,int port)
	{
		sockaddr_in ServerAddr;

		if(!Bind(s,port,ServerAddr))
		{
			return FALSE;
		}

		s = listen(s,CONNECTNUM);

		return s == 0;
	}
	static BOOL SendBuf( int s,char* buf,int len )
	{
		if (!len) 
			return FALSE;

		int nSize = 0;
		int nLeft = len;
		while (nLeft)
		{
			nSize = send(s,buf+(len-nLeft),nLeft,0);
			if (nSize == SOCKET_ERROR)
			{
				return FALSE;
			}
			nLeft-=nSize;
		}
		return TRUE;
	}
	static BOOL RecvBuf( int s,char* buf,int len )
	{
		if (!len) 
			return FALSE;

		int nSize = 0;
		int nLeft = len;
		while (nLeft)
		{
			nSize = recv(s,buf+(len-nLeft),nLeft,0);
			if (nSize == SOCKET_ERROR)
			{
				return FALSE;
			}
			nLeft-=nSize;
		}
		return TRUE;
	}
	static in_addr GetName(const char* name)
	{
		bool isFound = false;
		g_csCache.Enter();

		DNS_CACHE::iterator it = g_cache.find(name);
		if ( it != g_cache.end())
		{
			isFound = true;
		}

		g_csCache.Leave();

		if (isFound)
			return it->second;

		in_addr ret = { 0 };
#ifdef LINUX
		DNS::GetDns((char*)name,&ret);
#else
		addrinfo *info = NULL;
		addrinfo hint = {0};
		int nRet = getaddrinfo(name,NULL,&hint,&info);

		if (nRet == 0)
		{
			ret = ((sockaddr_in*)info->ai_addr)->sin_addr;
		}

#endif
		g_csCache.Enter();

		g_cache[name] = ret;

		g_csCache.Leave();

		return ret;
	}

	static void GetHostIP( LPSTR buf )
	{
		int ret;
		char name[100];
		addrinfo *info = NULL;
		addrinfo hint = {0};

		ret = gethostname(name,1024);

		if (ret != 0)
			return;

		in_addr HostAddr = GetName(name);
		memcpy(buf,(void*)&HostAddr.s_addr,4);
	}
	static void Close(SOCKET s)
	{
#ifdef LINUX
		shutdown(s,SHUT_RDWR);
#else
		closesocket(s);
#endif
	}
};
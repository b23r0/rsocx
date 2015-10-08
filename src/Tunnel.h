#pragma once
#include <set>
#include <map>
#include <iostream>

#include "socks/SocksParser.h"
#include "common/public.h"

#ifdef LINUX
	#define INVALID_SOCKET -1
#else
#endif

typedef std::set<SOCKET> SOCKET_SET;
typedef std::map<SOCKET,SOCKET> NEXUS_MAP;

typedef struct 
{
	SOCKET s1;
	SOCKET s2;
	LPVOID lpParameter;
}TUNNEL_CONFIG,*PTUNNEL_CONFIG;


class CTunnel
{
	DECLARE_SINGLETON(CTunnel)

public:

	BOOL Begin(int ,int);
	void Wait();
	void Close();
private:

	BOOL BindTunnel(int, int);

	BOOL WaitTunnel();

	static DWORD WINAPI Worker(LPVOID lpParameter);
	DWORD WINAPI WorkerProc();
	
	static DWORD WINAPI Tunnel(LPVOID lpParameter);
	DWORD WINAPI TunnelProc();

	static DWORD WINAPI TCPTunnel(LPVOID lpParameter);
	DWORD WINAPI TCPTunnelProc(LPVOID lpParameter);

	static DWORD WINAPI TCP_C2S(LPVOID );

	static DWORD WINAPI TCP_S2C(LPVOID );

	static DWORD WINAPI CheckMgr(LPVOID);
	DWORD WINAPI CheckMgrProc(LPVOID);
	
public:

	SOCKET m_s2;
	SOCKET m_s1;
	SOCKET m_sMgr;
	
	int m_port;
};


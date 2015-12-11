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

typedef std::set<int> SOCKET_SET;
typedef std::map<int,int> NEXUS_MAP;

typedef struct 
{
	int s1;
	int s2;
	LPVOID lpParameter;
}TUNNEL_CONFIG,*PTUNNEL_CONFIG;


class CTunnel
{
	DECLARE_SINGLETON(CTunnel)

public:

	bool Begin(int ,int);
	void Wait();
	void Close();
private:

	bool BindTunnel(int, int);

	bool WaitTunnel();

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

	int m_s2;
	int m_s1;
	int m_sMgr;
	
	int m_port;
};


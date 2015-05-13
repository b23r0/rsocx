#pragma once
#include "../common/public.h"
#include "../common/Socket.h"
#include <iostream>
#include <map>
#include <string>

typedef std::map<std::string,std::string> DNS_MAP;

using namespace Socket;

enum CMDTYPE 
{
	SOCKS_CONNECT = 0x01,
	SOCKS_BIND,
	SOCKS_UDP
};


typedef struct
{
	SOCKET  socket;
	SOCKET  usocket;
	sockaddr_in caddr;
	sockaddr_in saddr;
	in_addr  ipaddr;
	unsigned short c_port;
	unsigned short sq;

	CMDTYPE type;
	SOCKET slocal;
	SOCKET sremote;
	LPVOID lpParameter;
}SERVICE_INFO,*PSERVICE_INFO;

class SocksParser
{
	DECLARE_SINGLETON(SocksParser)
private:
	volatile int m_socket;

	CriticalSection m_csDns;
	DNS_MAP m_dns;

public:
	BOOL Auth(int s,char* username,char* password,BOOL NeedAuth);
	BOOL UDPResponse(SERVICE_INFO& svc);
	BOOL TCPResponse( SERVICE_INFO& svc );
	BOOL GetRequest( SERVICE_INFO& svc );

};
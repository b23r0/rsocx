#include "stdafx.h"
#include "SocksMgr.h"

#include "../thread/ThreadArray.h"

CSocksMgr::CSocksMgr():
	m_pwd("cs"),
	m_user("cs"),
	m_rIp("127.0.0.1"),
	m_rPort(8000),
	m_NeedAuth(FALSE)
{
}

CSocksMgr::~CSocksMgr()
{

}

DWORD WINAPI CSocksMgr::TCP_S2C(void* lpParameter)
{
	SERVICE_INFO* pSvcInfo = (SERVICE_INFO*)lpParameter;
	char buffer[1024*4] = {0};

	while (TRUE)
	{
		int nCount = recv(pSvcInfo->sremote,buffer,1024*4,0);
		if (nCount == SOCKET_ERROR)
		{
			debugLog(_T("recv Error! %d"),WSAGetLastError());
			break;
		}
		BOOL bRet = Socket::SendBuf(pSvcInfo->slocal,buffer,nCount);

		if (!bRet)
		{
			debugLog(_T("send Error! %d"),WSAGetLastError());
			break;
			break;
		}
	}

	Socket::Close(pSvcInfo->sremote);
	Socket::Close(pSvcInfo->slocal);

	return 0;
}

DWORD WINAPI CSocksMgr::TCP_C2S(void* lpParameter)
{
	SERVICE_INFO* pSvcInfo = (SERVICE_INFO*)lpParameter;
	char buffer[1024*4] = {0};

	while (TRUE)
	{
		int nCount = recv(pSvcInfo->slocal,buffer,1024*4,0);
		if (nCount == SOCKET_ERROR)
		{
			debugLog(_T("recv Error! %d"),WSAGetLastError());
			break;
		}

		BOOL bRet = Socket::SendBuf(pSvcInfo->sremote,buffer,nCount);

		if(!bRet)
		{
			debugLog(_T("send Error! %d"),WSAGetLastError());
			break;
		}
	}

	Socket::Close(pSvcInfo->sremote);
	Socket::Close(pSvcInfo->slocal);
	return 0;
}
DWORD WINAPI CSocksMgr::TCPTunnel( LPVOID lpParameter )
{
	return CSocksMgr::GetInstanceRef().TCPTunnelProc(lpParameter);
}

DWORD WINAPI CSocksMgr::TCPTunnelProc( LPVOID lpParameter )
{
	SERVICE_INFO* pSvcInfo = (SERVICE_INFO*)lpParameter;

	Thread t1 , t2;

	t1.Start((LPTHREAD_START_ROUTINE)TCP_C2S,pSvcInfo);
	t2.Start((LPTHREAD_START_ROUTINE)TCP_S2C, pSvcInfo);

	t1.WaitForEnd();
	t2.WaitForEnd();


	debugLog(_T("Tunnel thread finish!"));
	if (pSvcInfo)
	{
		free(pSvcInfo);
	}

	return TRUE;
}

BOOL CSocksMgr::Proxy( SOCKET s,LPSTR user , LPSTR pwd )
{
	SERVICE_INFO *pSvc = new SERVICE_INFO;
	pSvc->socket = s;

	BOOL ret = FALSE;

	do 
	{
		ret = SocksParser::GetInstanceRef().Auth(s ,user,pwd,m_NeedAuth);

		if ( ! ret )
			break;

		ret = SocksParser::GetInstanceRef().GetRequest(*pSvc);

		if ( ! ret )
			break;

		if (pSvc->type == SOCKS_UDP)
		{
			ret = SocksParser::GetInstanceRef().UDPResponse(*pSvc);
		}
		else
		{
			ret = SocksParser::GetInstanceRef().TCPResponse(*pSvc);

			if ( ! ret )
				break;

			//进入纯转发模式
			ret = m_threadList.AddThreadTask((LPTHREAD_START_ROUTINE)TCPTunnel,pSvc);
		}

	} while (FALSE);

	if ( !ret )
	{
		debugLog(_T("Proxy Error!"));
		delete pSvc;
	}

	return ret;

}

DWORD WINAPI CSocksMgr::Forward(void* lpParameter)
{
	return CSocksMgr::GetInstanceRef().ForwardProc(lpParameter);
}

DWORD WINAPI CSocksMgr::ForwardProc(void* lpParameter)
{
	SOCKET s = (SOCKET)lpParameter;

	BOOL ret = Proxy(s,(LPSTR)m_user.c_str(),(LPSTR)m_pwd.c_str());

	if ( !ret )
		Socket::Close(s);

	return 0;
}

DWORD WINAPI CSocksMgr::Redirect( LPVOID lpParameter )
{
	return CSocksMgr::GetInstanceRef().RedirectProc(lpParameter);
}

DWORD WINAPI CSocksMgr::RedirectProc( LPVOID lpParameter )
{
	PROXY_CONFIG* config = (PROXY_CONFIG*)lpParameter;

	SOCKET slocal = Socket::Create();
	SOCKET sremote = Socket::Create();

	BOOL ret = FALSE;

	do 
	{
		ret = Socket::Connect(sremote,m_rIp.c_str(),m_rPort);

		if ( !ret )
			break;

		ret = Socket::Connect(slocal,config->ip,config->port);

		if ( !ret )
			break;

		Socket::SendBuf(slocal,(char*)config,sizeof(PROXY_CONFIG));

		SERVICE_INFO *pSvc = new SERVICE_INFO;
		pSvc->slocal = slocal;
		pSvc->sremote = sremote;

		//进入纯转发模式
		ret = m_threadList.AddThreadTask((LPTHREAD_START_ROUTINE)TCPTunnel,pSvc);

	} while (FALSE);

	if (config)
	{
		free(config);
	}

	return ret;
}

DWORD WINAPI CSocksMgr::Reverse(void* lpParameter)
{
	return CSocksMgr::GetInstanceRef().ReverseProc(lpParameter);
}

DWORD WINAPI CSocksMgr::ReverseProc(void* lpParameter)
{
	PROXY_CONFIG* config = (PROXY_CONFIG*)lpParameter;
	SOCKET s = Socket::Create();

	BOOL ret = FALSE;

	do 
	{
		ret = Socket::Connect(s,config->ip,config->port);

		if ( !ret )
			break;

		Socket::SendBuf(s,(char*)config,sizeof(PROXY_CONFIG));

		ret = Proxy(s,config->user,config->pwd);

	} while (FALSE);

	if ( !ret )
	{
		Socket::Close(s);
	}

	if (config)
	{
		free(config);
	}

	return ret;
}


BOOL CSocksMgr::Begin( LPCSTR ip1, int port1,LPCSTR ip2,int port2)
{
	m_rIp = ip2;
	m_rPort = port2;

	SOCKET s = Socket::Create();

	if (s == SOCKET_ERROR)
		return 0;

	infoLog(_T("Connecting %s:%d"),a2t(ip1),port1);

	if(!Socket::Connect(s,ip1,port1))
	{
		errorLog(_T("Connect Faild!"));
		return FALSE;
	}

	infoLog(_T("Connect Success!"));


	BOOL ret = FALSE;
	PROXY_CONFIG* proxy = new PROXY_CONFIG;

	do
	{
		ret = RecvBuf(s,(char*)proxy,sizeof(PROXY_CONFIG));

		if ( !ret )
			break;

		strncpy(proxy->ip,ip1,20);
		strncpy(proxy->user,m_user.c_str(),20);
		strncpy(proxy->pwd,m_pwd.c_str(),20);

		proxy->lpParameter = (uint32_t)this;
		m_threadList.AddThreadTask((LPTHREAD_START_ROUTINE)Redirect,proxy);

		proxy = new PROXY_CONFIG;

	}while(TRUE);

	errorLog(_T("Disconnect!"));

	return ret;
}

BOOL CSocksMgr::Begin( LPCSTR ip, int port )
{
	SOCKET s = Socket::Create();

	if (s == SOCKET_ERROR)
		return 0;

	infoLog(_T("Connecting %s:%d"),a2t(ip),port);

	if(!Socket::Connect(s,ip,port))
	{
		errorLog(_T("Connect Faild!"));
		return FALSE;
	}

	infoLog(_T("Connect Success!"));

	BOOL ret = FALSE;
	PROXY_CONFIG* proxy = new PROXY_CONFIG;

	do
	{
		ret = RecvBuf(s,(char*)proxy,sizeof(PROXY_CONFIG));

		if ( !ret )
			break;

		strncpy(proxy->ip,ip,20);
		strncpy(proxy->user,m_user.c_str(),20);
		strncpy(proxy->pwd,m_pwd.c_str(),20);

		proxy->lpParameter = (uint32_t)this;
		m_threadList.AddThreadTask((LPTHREAD_START_ROUTINE)Reverse,proxy);

		proxy = new PROXY_CONFIG;

	}while(TRUE);

	errorLog(_T("Disconnect!"));

	return ret;
}

BOOL CSocksMgr::Begin( int port )
{
	SOCKET s = Socket::Create();

	if (s == SOCKET_ERROR)
		return 0;

	BOOL ret = FALSE;
	do 
	{
		
		if ( ! Socket::Listen(s ,port))
		{
			errorLog(_T("Bind %d faild!"),port);
			break;
		}

		infoLog(_T("Listening %d"),port);

		sockaddr_in raddr;

		while (TRUE)
		{
			SOCKET rs = Socket::Accept(s,(sockaddr*)&raddr);

			if (rs == SOCKET_ERROR)
			{
				errorLog(_T("Accept faild!"),port);
				ret = FALSE;
				break;
			}

			infoLog(_T("Accept : %s"),a2t(inet_ntoa(raddr.sin_addr)));
			
			m_threadList.AddThreadTask((LPTHREAD_START_ROUTINE)Forward,(void*)rs);
		}


	} while (FALSE);

	errorLog(_T("Disconnect!"));

	return ret;
}

void CSocksMgr::Close()
{
}

void CSocksMgr::Wait()
{
}

void CSocksMgr::SetAuth( LPCTSTR user,LPCTSTR pwd )
{
	m_user = t2a(user);
	m_pwd = t2a(pwd);
	m_NeedAuth = TRUE;
}
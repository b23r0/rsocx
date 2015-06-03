#include "stdafx.h"
#include <map>
#include <string>
#include "Tunnel.h"
#include "th3rd/dns.h"
#include "socks/SocksMgr.h"

using namespace std;

#ifdef LINUX
typedef map<string,string> CMD_MAP;
#else
typedef map<wstring,wstring> CMD_MAP;
#endif

void Version()
{
	return;
}

void Usage()
{
	printf("\nUsage    : xsocks  [-l port] [-t] [-p1 port] [-p2 port] [-s ip:port] \n");
	printf("                   [-r ip:port] [-u username] [-p password]          \n\n");

	printf("Options  : -l  Set forward mode on Socks5.                              \n");
	printf("           -r  Set reverse mode on Socks5.                              \n");
	printf("           -t  Build tunnel on socks5.                                  \n");
	printf("           -s  Redirect another socks server.                          \n");
	printf("           -u  Socks5's login username.                                \n");
	printf("           -p  Socks5's login password.                                \n");
	printf("           -p1 Accept of XSOCKS client's port.                         \n");
	printf("           -p2 Accept of proxy  client's port.                       \n\n");

	printf("Examples : xsocks -l 8085 -u root -p 123456                            \n");
	printf("           xsocks -t -p1 8085 -p2 8086                                 \n");
	printf("           xsocks -r 192.168.1.10:8085 -u root -p 123456               \n");
	printf("           xsocks -s 192.168.1.11:8085 -r 192.168.1.10:8086          \n\n");
}
#ifdef LINUX
void LoadCommand(int argc, char* argv[] ,CMD_MAP& map)
#else
void LoadCommand(int argc, _TCHAR* argv[] ,CMD_MAP& map)
#endif
{
	for(int i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			if (i+1 != argc && argv[i+1][0] != '-')
			{
				map[argv[i]] = argv[i+1];
			}
			else
			{
				map[argv[i]] = _T("");
			}
		}
	}
}

#ifdef LINUX
int main(int argc, char* argv[])
{
	string user = _T("");
	string pwd = _T("");
#else
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	wstring user = _T("");
	wstring pwd = _T("");
#endif

	CMD_MAP cmd;

	LoadCommand(argc,argv,cmd);
	BOOL ret = FALSE;

	do 
	{
		//获取验证账号和密码

		CMD_MAP::iterator it = cmd.find(_T("-p"));
		if ( it != cmd.end())
			pwd = it->second;

		it = cmd.find(_T("-u"));
		if ( it != cmd.end())
			user = it->second;

		if (user.length() != 0 && pwd.length() != 0)
			CSocksMgr::GetInstanceRef().SetAuth(user.c_str(),pwd.c_str());


		//没有参数
		if (cmd.size() == 0)
		{
			Usage();
			ret = TRUE;
			break;
		}
#ifdef LINUX
// 		infoLog("Initialize DNS client....");
// 		if ( !DNS::InitDns() )
// 		{
// 			errorLog("Initialize DNS client faild!");
// 			ret = TRUE;
// 			break;
// 		}
#endif

		//反弹模式
		it = cmd.find(_T("-r"));

		if (it != cmd.end())
		{
			int split = it->second.find(':');

			if (split == -1)
			{
				break;
			}

			string ip = t2a(it->second.substr(0,split).c_str());
			int port  = atoi(t2a(it->second.substr(split+1,it->second.length()).c_str()));

			it = cmd.find(_T("-s"));

			if (it == cmd.end())
			{
				Version();
				CSocksMgr::GetInstanceRef().Begin(ip.c_str(),port);
				ret = TRUE;
			}
			else
			{
				int split = it->second.find(':');
				if (split == -1)
				{
					break;
				}
				string ip2 = t2a(it->second.substr(0,split).c_str());
				int port2  = atoi(t2a(it->second.substr(split+1,it->second.length()).c_str()));

				Version();
				CSocksMgr::GetInstanceRef().Begin(ip.c_str(),port,ip2.c_str(),port2);
				ret = TRUE;
			}

			break;
		}

		//隧道模式
		it = cmd.find(_T("-t"));

		if (it != cmd.end())
		{
			it = cmd.find(_T("-p1"));
			if (it == cmd.end() )
				break;

			int port1 = atoi(t2a(it->second.c_str()));

			it = cmd.find(_T("-p2"));
			if (it == cmd.end() )
				break;

			int port2 = atoi(t2a(it->second.c_str()));

			Version();
			CTunnel::GetInstanceRef().Begin(port1,port2);
			ret = TRUE;
			break;
		}

		//正向模式
		it = cmd.find(_T("-l"));

		if (it != cmd.end())
		{
			int port = atoi(t2a(it->second.c_str()));

			Version();
			CSocksMgr::GetInstanceRef().Begin(port);
			ret = TRUE;
			break;
		}



	} while (FALSE);

	if ( !ret )
	{
		Usage();
	}

	return 0;
}


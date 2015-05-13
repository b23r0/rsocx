#pragma once

#ifdef LINUX
	#include <stdio.h>
	#include <stdlib.h>
	#include <error.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

#else
#endif

#include <time.h>
#include <fstream>

namespace DNS
{
	void format_packet(unsigned char *buf,in_addr* addr);
	unsigned char *format_name(unsigned char *p,unsigned char *buf);
	bool InitDns();
	bool GetDns(char* argv,in_addr* addr);
};

#define GETWORD(__w,__p) do{__w=*(__p++)<<8;__w|=*(p++);}while(0)
#define GETLONG(__l,__p) do{__l=*(__p++)<<24;__l|=*(__p++)<<16;__l|=*(__p++)<<8;__l|=*(p++);}while(0)

#include "stdafx.h"
#include "dns.h"
#include <string>

static std::string g_dns = "8.8.8.8";

bool DNS::InitDns()
{
	std::fstream file;
	file.open("/etc/resolv.conf",std::ios::in);

	bool ret = false;

	if (file.is_open())
	{
		char *buf = (char*)malloc(5000);

		do{
			char* flag = "nameserver";

			file.read(buf,5000);

			std::string strOptions = buf;

			int pos = strOptions.find(flag);

			std::string ip = strOptions.substr(pos,strOptions.length() - pos).c_str();

			if (pos == -1)
				break;

			pos = ip.find("\n");
			int len = strlen(flag)+1;

			if (pos == -1)
			{
				ip = ip.substr(len,ip.length() - len);
			}
			else
			{
				ip = ip.substr(len,pos - len);
			}			

			g_dns = ip;

			infoLog("DNS Server Address : %s",ip.c_str());

			ret = true;

		}while(false);
		
		if (buf)
			free(buf);

		file.close();
	}

	return ret;
}

bool DNS::GetDns(char* argv,in_addr* addr)
{
	time_t ident;
	int fd;
	int rc;
	socklen_t serveraddrlent;
	char *q;
	unsigned char *p;
	unsigned char *countp;
	unsigned char reqBuf[512] = {0};
	unsigned char rplBuf[512] = {0};
	sockaddr_in serveraddr;
 

	bool ret = false;

	do 
	{
		//udp
		fd = socket(AF_INET, SOCK_DGRAM, 0);

		//set timeout
#ifdef LINUX
		timeval tv_out;
		tv_out.tv_sec = 5;
		tv_out.tv_usec = 0;
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv_out,sizeof(timeval));

#else
		int outtime = 5000;
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&outtime,sizeof(int));

#endif

		if(fd == -1)
		{
			/*	perror("error create udp socket");*/
			break;
		}

		time(&ident);
		//copy
		p = reqBuf;
		//Transaction ID
		*(p++) = ident;
		*(p++) = ident>>8;
		//Header section
		//flag word = 0x0100
		*(p++) = 0x01;
		*(p++) = 0x00;
		//Questions = 0x0001
		//just one query
		*(p++) = 0x00;
		*(p++) = 0x01;
		//Answer RRs = 0x0000
		//no answers in this message
		*(p++) = 0x00;
		*(p++) = 0x00;
		//Authority RRs = 0x0000
		*(p++) = 0x00;
		*(p++) = 0x00;
		//Additional RRs = 0x0000
		*(p++) = 0x00;
		*(p++) = 0x00;
		//Query section
		countp = p;  
		*(p++) = 0;
		for(q=argv; *q!=0; q++)
		{
			if(*q != '.')
			{
				(*countp)++;
				*(p++) = *q;
			}
			else if(*countp != 0)
			{
				countp = p;
				*(p++) = 0;
			}
		}
		if(*countp != 0)
			*(p++) = 0;

		//Type=1(A):host address
		*(p++)=0;
		*(p++)=1;
		//Class=1(IN):internet
		*(p++)=0;
		*(p++)=1;

		//	 printf("\nRequest:\n");
		//	 printmessage(reqBuf);

		//fill
		memset((void*)&serveraddr,0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(53);
		serveraddr.sin_addr.s_addr = inet_addr(g_dns.c_str());

		//send to DNS Serv
		if(sendto(fd,(char*)reqBuf,p-reqBuf,0,(sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
		{
			/*		perror("error sending request");*/
			break;
		}

		//recev the reply
		memset((void*)&serveraddr,0, sizeof(serveraddr));
		serveraddrlent = sizeof(serveraddr);

		rc = recvfrom(fd,(char*)rplBuf,sizeof(rplBuf),0,(sockaddr *)&serveraddr,&serveraddrlent);
		if(rc < 0)
		{
			/*		perror("error receiving request\n");*/
			break;
		}

		//print out results
		/*	printf("\nReply:\n");*/
		format_packet(rplBuf,addr);

		ret = true;

	} while (false);
	
	return ret; 
}

void DNS::format_packet(unsigned char *buf,in_addr* addr)
{
	unsigned char *p;
	unsigned int ident,flags,qdcount,ancount,nscount,arcount;
	unsigned int i,type,cls,ttl,rdlength;
 
	p = buf;
	GETWORD(ident,p);
//	printf("ident=%#x\n",ident);
 
	GETWORD(flags,p);
//	 printf("flags=%#x\n",flags);
//	 //printf("qr=%u\n",(flags>>15)&1);
//	 printf("qr=%u\n",flags>>15);
 
//	 printf("opcode=%u\n",(flags>>11)&15);
//	 printf("aa=%u\n",(flags>>10)&1);
//	 printf("tc=%u\n",(flags>>9)&1);
//	 printf("rd=%u\n",(flags>>8)&1);
//	 printf("ra=%u\n",(flags>>7)&1);
//	 printf("z=%u\n",(flags>>4)&7);
//	 printf("rcode=%u\n",flags&15); 
 
	GETWORD(qdcount,p);
/*	printf("qdcount=%u\n",qdcount);*/
 
	GETWORD(ancount,p);
/*	printf("ancount=%u\n",ancount);*/
 
	GETWORD(nscount,p);
/*	printf("nscount=%u\n",nscount);*/
 
	GETWORD(arcount,p);
/*	printf("arcount=%u\n",arcount);*/
 
	for(i=0; i<qdcount; i++)
	{
/*		printf("qd[%u]:\n",i);*/
		while(*p!=0)
		{
			p = format_name(p,buf);
/*			if(*p != 0)*/
/*			  printf(".");*/
		}
		p++;
/*		printf("\n");*/
		GETWORD(type,p);
/*		printf("type=%u\n",type);*/
		GETWORD(cls,p);
/*		printf("class=%u\n",class);*/
	}
 
	for(i=0; i<ancount; i++)
	{
/*		printf("an[%u]:\n",i);*/
		p = format_name(p,buf);
/*		printf("\n");*/
		GETWORD(type,p);
/*		printf("type=%u\n",type);*/
		GETWORD(cls,p);
/*		printf("class=%u\n",class);*/
		GETLONG(ttl,p);
/*		printf("ttl=%u\n",ttl);*/
		GETWORD(rdlength,p);
/*		printf("rdlength=%u\n",rdlength);*/
/*		printf("rd=");*/
		if (rdlength == 4)
		{
			memcpy(&addr->s_addr,p,4);
			break;
		}
		p += rdlength;
	}
}
 
unsigned char *DNS::format_name(unsigned char *p,unsigned char *buf)
{
	unsigned int nchars,offset;
 
	nchars = *(p++);
	if((nchars & 0xc0) == 0xc0)
	{
		offset = (nchars & 0x3f) << 8;
		offset |= *(p++);
		nchars = buf[offset++];
/*		printf("%*.*s",nchars,nchars,buf+offset);*/
	}
	else
	{
/*		printf("%*.*s",nchars,nchars,p);*/
		p += nchars;
	}
 
	return (p);
}
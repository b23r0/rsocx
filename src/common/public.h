#pragma once

#ifndef LINUX
typedef int uint32_t;
#endif

typedef struct
{
	uint32_t s;
	uint32_t port;
	char ip[20];
	char user[20];
	char pwd[20];
	uint32_t  lpParameter;
}PROXY_CONFIG,*PPROXY_CONTROL;

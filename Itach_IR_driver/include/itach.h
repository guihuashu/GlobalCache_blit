#ifndef _ITACH_H
#define _ITACH_H

#include <conf.h>
#include <list.h>
#include <stdbool.h>


#define UUID_LEN 25;
#define IP_LEN 16;

typedef struct itach{
	char ip[16];
	char uuid[25];
	list_t list;
	int tcpfd;			// 主动跟itach模块tcp连接成功后返回的客户端套接字
}itach_t;


extern list_t g_itach_head;
extern pthread_mutex_t g_itachs_mutex;

itach_t *get_itach(const char *uuid);
void scanning_itachs();
#endif // _ITACH_H

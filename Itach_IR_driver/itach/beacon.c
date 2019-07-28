#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <debug.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <public_opr.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include <itach.h>
#include <conf.h>
#include <public_opr.h>
#include <cJSON.h>
#include <IRdev.h>
#include <signal.h>
#include <unistd.h>
#include <list.h>
#include <debug.h>
#include <stdbool.h>



void get_uuid_from_beacon(char *beacon, char *uuid)
{
	char *ptr2 = strstr(beacon, "><-SDKClass");
	char *ptr1 = strstr(beacon, "UUID=") + strlen("UUID=");

	int uuid_len = ptr2 - ptr1;
	strncpy(uuid, ptr1, uuid_len);
	uuid[uuid_len] = '\0';
}

void get_ip_from_beacon(char *beacon, char *ip)
{
	char *ptr2 = strstr(beacon, "><-PCB_PN");
	char *ptr1 = strstr(beacon, "//") + strlen("//");

	int iplen = ptr2 - ptr1;
	strncpy(ip, ptr1, iplen);
	ip[iplen] = '\0';
}


int beacon_init()
{
	// 本地ip绑定9131端口, 加入组播"239.255.250.250"
	struct sockaddr_in native_addr;
	struct ip_mreq mreq;
	int udpfd;
	memset(&native_addr, 0, sizeof(native_addr));
	native_addr.sin_port		= htons(BEACON_MULTI_PORT); 			
	native_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	udpfd = socket(AF_INET, SOCK_DGRAM,0);
	if (udpfd <= 0)	
		return -1;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(BEACON_MULTI_ADDR); // 组播地址是D类地址
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);	 // 客户端网卡地址//或    inet_addr("客户端网卡地址");
	setsockopt(udpfd, IPPROTO_IP,IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

	if (-1 == bind(udpfd, (struct sockaddr *)&native_addr, sizeof(native_addr))){
		close(udpfd);
		return -1;
	}
	return udpfd;
}




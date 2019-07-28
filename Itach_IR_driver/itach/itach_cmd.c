#include <itach.h>
#include <conf.h>
#include <public_opr.h>
#include <IRdev.h>
#include <signal.h>
#include <unistd.h>
#include <list.h>
#include <debug.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <comm_gateway.h>

void connect_to_itach(itach_t *new_itach)
{
	struct sockaddr_in itach_server_addr;

	memset(&itach_server_addr, 0, sizeof(itach_server_addr));
	itach_server_addr.sin_family		=	AF_INET;
	itach_server_addr.sin_port			=	htons(ITACH_TCP_PORT);	//4998
	itach_server_addr.sin_addr.s_addr	=	inet_addr(new_itach->ip);	
	
	
	int tcpfd = socket(AF_INET,SOCK_STREAM,0);	// 作为tcp客户端进行连接
	if (tcpfd <= 0)
		goto err;
	if (-1 == connect(tcpfd, (struct sockaddr *)&itach_server_addr, sizeof(itach_server_addr)))
		goto err;
	new_itach->tcpfd = tcpfd;
	
	return;
	
err:
	DBG("ERROR: connect_to_itach\n");
	exit(0);
}

void reconnection_itach(itach_t *itach)
{
	close(itach->tcpfd);
	connect_to_itach(itach);
}


static bool tcp_send_cmd(int fd, const char *cmd)
{
	int ret;
	int len = strlen(cmd);
	
	ret = send(fd, cmd, len, 0);
	return (ret == len);
}

static bool tcp_recv_reply(int fd, char *buf, int len)
{
	int ret;
	
	ret = recv(fd, buf, len, 0);
	printf("recv:%s\n", buf);
	if (ret <= 0) {
		DBG("connect err, now reconnect..\n");
		return false;
	}
	return true;
}


static bool replay_ok(const char *reply)
{
	if (strstr(reply, "completeir"))
		return true;
	else if(strstr(reply, "ERR"))
		return false;
	else {
		DBG("unknown tcp cmd replay:%s\n", reply);
		return false;
	}
}

/* 发送错误返回示例:  	     ERR IR005
 * 发送成功返回示例: completeir,1:1,1
 * 红外码示例:
 	sendir,1:1,1,38000,1,69,341,170,21,64,21,64,21,64,21,21,21,21,21,21,21,21,21,64,21,64,21,21,21,64,21,21,21,21,21,21,21,64,21,21,21,21,21,21,21,64,21,21,21,21,21,64,21,21,21,21,21,64,21,64,21,21,21,64,21,64,21,21,21,64,21,64,21,1517,341,85,21,3655
 */
bool tcp_send_IRcode(write_packet_t *packet)
{
	CUR;
	char reply[100];
	char *IRcode;

	
	IRdev_t *IRdev = get_IRdev(packet->sid);
	itach_t *itach = get_itach(IRdev->uuid);
	if (!IRdev || !itach || (packet->value < 1)) {
		DBG("ERROR: tcp_send_IRcode\n");
		return false;
	}
	/* json文件里面指定了红外码发送的通道号, 用这个通道号替换原红外码的通道号 */
	IRcode = find_IRCode(IRdev, packet->value -1);
	if (!IRcode) {
		DBG("cannot find IRcode\n");
		exit(0);
	}
	//printf("IRcode:\n%s\n", IRcode);
	//return false;

	int len = strlen(IRcode) + 3;
	char send_IRcode[len];	// 必须回车符(\r\n)才能发送成功, 看tcp文档
	memset(send_IRcode, 0, len);
	strcpy(send_IRcode, IRcode);	//从第0项开始存放红外码
	memset(reply, 0, 100);
#if 0
	//send_IRcode[strlen("sendir,1:")] = IRdev->ChannelNo + 48;	//  + 48: int转换为字符
#endif
	send_IRcode[len-3] = '\r';
	send_IRcode[len-2] = '\n';
	send_IRcode[len-1] = '\0';
	printf("\nfd=%d\n, ip= %s, \nIRcode:%s\n", itach->tcpfd, itach->ip, send_IRcode);
	if (!tcp_send_cmd(itach->tcpfd, send_IRcode)) {
		DBG("\nERROR: tcp_send_cmd, reconnection_itach\n");
		reconnection_itach(itach);
		return false;
	}
	if(!tcp_recv_reply(itach->tcpfd, reply, 100))  {
		DBG("\nERROR: tcp_recv_reply, reconnection_itach\n");
		reconnection_itach(itach);
		return false;
	}
	if (!replay_ok(reply)) {
		return false;
	}
	return true;
}



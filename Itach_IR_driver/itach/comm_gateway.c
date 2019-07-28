#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <fcntl.h>
#include <pthread.h>

#include <public_opr.h>
#include <conf.h>
#include <debug.h>
#include <cJSON.h>
#include <IRdev.h>
#include <comm_gateway.h>
#include <itach_http.h>
#include <status_file.h>
#include <itach_cmd.h>

static unsigned long int cnt = 0;

#define GET_SID(uid, sid) \
	memset(uid, 0, 9);\
	strncpy(uid, sid, 8);	\
	uid[8] = '\0'

static int driver_fd;
static int sockaddr_un_len;
static struct sockaddr_un recvfrom_addr; 
static struct sockaddr_un sendto_addr; 

// 通讯网关--接收初始化
void comm_gw_recvfrom_init()
{

	// udp域套接字
	driver_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (driver_fd <= 0)
		goto err;
	sockaddr_un_len = sizeof(struct sockaddr_un);
	memset(&recvfrom_addr, 0, sockaddr_un_len);
	recvfrom_addr.sun_family = AF_UNIX;	
	strcpy(recvfrom_addr.sun_path, RECVFROM_GW_PIPE); 

	// 删除存在的管道文件, 否则绑定失败
	if (file_existed(RECVFROM_GW_PIPE)) 
		rm_file(RECVFROM_GW_PIPE);
	
	// 绑定
	if (-1 == bind(driver_fd, (struct sockaddr *)&recvfrom_addr, sockaddr_un_len)) {
		goto err;
	}
#if 0
	int flags = fcntl(driver_fd, F_GETFL);
	flags |= O_NONBLOCK;
	if(fcntl(driver_fd, F_SETFL, flags) < 0)
	{
		 perror("fcntl err\n");
		 exit(0);
	}
#endif	
	return;
	
err:
	DBG("bind err\n");
	exit(0);
}
void comm_gw_sento_init()
{
	memset(&sendto_addr, 0, sockaddr_un_len);
	sendto_addr.sun_family = AF_UNIX;	
	strcpy(sendto_addr.sun_path, SENDTO_GW_PIPE); 
}

void comm_gw_init()
{
	comm_gw_recvfrom_init();
	
	comm_gw_sento_init();
	DBG("comm_gw_init ok\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
static int sendto_gw(const char *buf, const int len)
{
	int addrlen = sizeof(struct sockaddr_un);	// 既是输入值又是输出值
	return sendto(driver_fd, buf, len, 0, (struct sockaddr *)&sendto_addr, addrlen);
}
static int recvfrom_gw(char *buf, const int len)
{
	return recvfrom(driver_fd, buf, len, 0, NULL, NULL);
}
	
/*{
	"cmd": "connect",
	"name": "&(驱动名称)",
	"code":"&(驱动代码)"
}*/
static void send_connect_cmd()
{
	cJSON* obj = cJSON_CreateObject();  
	cJSON_AddStringToObject(obj, "cmd", "connect");  
	cJSON_AddStringToObject(obj, "name", ITACH_DRIVER_NAME);  
	cJSON_AddStringToObject(obj, "code", ITACH_DRIVER_CODE);  
	char *sendbuf = cJSON_Print(obj);

	sendto_gw(sendbuf, strlen(sendbuf));
	free(sendbuf);
	cJSON_Delete(obj);
}
static bool is_connected_gw(const char *gw_reply)
{
	cJSON* obj = cJSON_Parse(gw_reply);
	char *cmd = cJSON_GetObjectItem(obj, "cmd")->valuestring;
	char *name = cJSON_GetObjectItem(obj, "name")->valuestring;
	
	if (!strcmp(cmd, "connect_ack") && !strcmp(name, "infrared")) {
		cJSON_Delete(obj);
		return true;
	}
	cJSON_Delete(obj);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
static void send_heartbeat_pakage()
{
	cJSON* obj = cJSON_CreateObject();  
	cJSON_AddStringToObject(obj, "cmd", "heartbeat");  
	char *sendbuf = cJSON_Print(obj);

	//printf("%s\n", sendbuf);
	int len = strlen(sendbuf);
	if (0 > sendto_gw(sendbuf, len))
		DBG("ERROR: send_heartbeat_pakage");
	free(sendbuf);
	cJSON_Delete(obj);
}

#if 0
static bool heart_reply_ok(const char *gw_reply)
{
	cJSON* obj = cJSON_Parse(gw_reply);
	char *cmd = cJSON_GetObjectItem(obj, "cmd")->valuestring;
	
	if (!strcmp(cmd, "heartbeat_ack")) {
		cJSON_Delete(obj);
		return true;
	}
	cJSON_Delete(obj);
	return false;
}
#endif

static void *host_heartbeating(void       *args)
{
	pthread_detach(pthread_self());	
	while(1) {
		send_heartbeat_pakage();
		sleep(HEARTBEAT_CYCLE);	// 20s
	}
	return NULL;
}

void connect_to_gateway()	
{
	char buf[100];

	// 连接到网关
	comm_gw_init();
	send_connect_cmd();
	memset(buf, 0, sizeof(buf));
	CUR;
	recvfrom_gw(buf,sizeof(buf));
	CUR;
#if 1	
	if (!is_connected_gw(buf)) {
		DBG("ERROR: conect to gw\n");
		exit(0);
	}
#endif
	DBG("connect to gateway ok...\n");
}

void sending_hostheart_packet()
{
	pthread_t uid;
	pthread_create(&uid, NULL, host_heartbeating, NULL);
}
//////////////////////////////////////////////////////////////////////////////////

void upload_IRdev_status(IRdev_t *IRdev, const int value)
{
	printf("%s %d\n", IRdev->sid, value);
	//char buf[100];
	cJSON *obj 		= cJSON_CreateObject();
	cJSON_AddStringToObject(obj, "cmd", "status");
	cJSON *device 	= cJSON_CreateObject();
	cJSON_AddItemToObject(obj, "device", device);
	cJSON *objects 	= cJSON_CreateArray();
	cJSON_AddItemToObject(device, "objects", objects);
	cJSON *item		= cJSON_CreateObject();
	cJSON_AddItemToArray(objects, item);

	cJSON_AddStringToObject(item, "sid", IRdev->sid);
	cJSON_AddNumberToObject(item, "value", value);
	
	char *str = cJSON_Print(obj);
	
	sendto_gw(str, strlen(str));
	cJSON_Delete(obj);
	free(str);
	
	return;
#if 0
	/* 上报后读取返回信息 */
	memset(buf, 0, sizeof(buf));
	recvfrom_gw(buf, sizeof(buf));
	cJSON *reply_obj = cJSON_Parse(buf);

	//DBG("reply_obj:\n%s\n", buf);
	
	if (!reply_obj) 
		goto err;
	if (strcmp("status_ack", cJSON_GetObjectItem(reply_obj, "cmd")->valuestring)
			|| (0 != cJSON_GetObjectItem(reply_obj, "return")->valueint))
		goto err;
	free(str);
	cJSON_Delete(obj);
	cJSON_Delete(reply_obj);
	DBG("\n\n-----------------------------------------------\nupload IRdev status ok:\n");
	DBG("value = %d\n", value);
	print_IRdev(IRdev);
#endif

}

void upload_init_IRdevs_status()
{
	list_t *pos;
	IRdev_t *IRdev;
	pthread_mutex_lock(&g_IRdevs_mutex);
	
	list_for_each(pos, &g_IRdevs_head){
		IRdev = container_of(pos, IRdev_t, list);
		upload_IRdev_status(IRdev, IRdev->status_value);
		DBG("upload Rdevs status ok\n");
	}
	pthread_mutex_unlock(&g_IRdevs_mutex);
}

void insert_write_packet(list_t *head, const char *sid, int value)
{
	write_packet_t *packet = malloc(sizeof(write_packet_t));
	if (!packet) {
		DBG("ERROR: insert_write_packet\n");
		exit(0);
	}
	packet->value = value;
	bzero(packet->sid, sizeof(packet->sid));
	strcpy(packet->sid, sid);
	list_add_tail(&packet->list, head);
}

static bool parse_write_packet(cJSON *obj, list_t *head)
{
	int i;
	int value;
	char *sid;
	cJSON *objects_i;
	cJSON *device = cJSON_GetObjectItem(obj, "device");
	cJSON *objects = cJSON_GetObjectItem(device, "objects");
	int size = cJSON_GetArraySize(objects);
	for (i=0; i < size; i++){
		objects_i = cJSON_GetArrayItem(objects, i);
		sid = cJSON_GetObjectItem(objects_i, "sid")->valuestring;
		value = cJSON_GetObjectItem(objects_i, "value")->valueint;
		if (value <= 0){	// value指定取红外码数组的第几项, 所以value必须>=0
			DBG("value must >= 0\n");
			return false;
		}
		insert_write_packet(head, sid, value);
		CUR;
		printf("sid = %s\nvalue = %d\n",sid, value);
	}
	return true;
}


static void free_write_packet_list(list_t *head)
{
	list_t *pos, *p;
	write_packet_t *packet;
	list_for_each_prev_safe(pos, p, head) {
		packet = container_of(pos, write_packet_t, list);
		list_del(pos);
		free(packet);
	}
}



static void send_write_packet(list_t *write_packet_head)
{
	list_t *pos;
	int cnt_;
	CUR;
	
	list_for_each(pos, write_packet_head){
		write_packet_t *packet;
		IRdev_t *IRdev;
		packet = container_of(pos, write_packet_t, list);
		if (!packet) {
			DBG("ERROR: packet is null\n");
			return;
		}
		printf("sid = %s\nvalue = %d\n", packet->sid, packet->value);
		cnt_ = 1;			// 发送次数
retry:
		if (tcp_send_IRcode(packet)) {
			IRdev = get_IRdev(packet->sid);
			upload_IRdev_status(IRdev, packet->value);
			record_status(IRdev->sid, packet->value);
			DBG("\nOK: tcp_send_IRcode send.............\n");
			continue;
		}
		else if (++cnt_ > 3)	{
			DBG("ERROR: tcp_send_IRcode\n");
			continue;
		}
		else{
			DBG("retry: tcp_send_IRcode\n");
			goto retry;
		}
	
	// http拿到的红外码有问题, 没有指明post的频率.重复次数等信息	
	}
}



static void upload_IRdev_offline(IRdev_t *IRdev)
{
	cJSON *obj = cJSON_CreateObject();
	char uid[9];	// 1byte用2位16进制表示
	// 获取uid
	GET_SID(uid, IRdev->sid);
	cJSON_AddStringToObject(obj, "cmd", "online_ack");
	cJSON_AddStringToObject(obj, "uid", uid);
	cJSON_AddStringToObject(obj, "status", "offline");
	char *str = cJSON_Print(obj);
	sendto_gw(str, strlen(str));
	free(str);
	cJSON_Delete(obj);
}



// 上报itach模块掉线(上报itach模块上连接的所有设备)
void upload_itach_lost(itach_t *itach)
{
	list_t *pos;
	IRdev_t *IRdev;
	pthread_mutex_lock(&g_IRdevs_mutex);
	list_for_each(pos, &g_IRdevs_head){
		IRdev = container_of(pos, IRdev_t, list);
		if (!strcmp(IRdev->uuid, itach->uuid)) {
			upload_IRdev_offline(IRdev);
		}
	}
	pthread_mutex_unlock(&g_IRdevs_mutex);
}



static bool this_is_host_heartbeat(cJSON *obj)
{
	cJSON *cmd;
	if (!(cmd = cJSON_GetObjectItem(obj, "cmd")))
		return false;
	return !strcmp("heartbeat", cmd->valuestring);
}
static bool this_is_write_packet(cJSON *obj)
{
	cJSON *write;
	if (!(write = cJSON_GetObjectItem(obj, "cmd")))
		return false;
	return !strcmp("write", write->valuestring);
}
void heartbeat_ack()
{
	cJSON *obj;
	char *buf;

	obj = cJSON_CreateObject();
	cJSON_AddStringToObject(obj, "cmd", "heartbeat_ack");
	buf = cJSON_Print(obj);
	if (sendto_gw(buf, strlen(buf)) <= 0) {
		DBG("ERROR: heartbeat_ack\n");
		exit(0);
	}
	DBG("send\n%s\n", buf);
	free(buf);
	cJSON_Delete(obj);
}

void comm_gw()
{
	while(1)
	{
		char buf[500];
		cJSON *obj;
		list_t packet_head;
		
		memset(buf, 0, sizeof(buf));
		INIT_LIST_HEAD(&packet_head);
		recvfrom_gw(buf, sizeof(buf));
	
		printf("%lu\n%s\n", ++cnt, buf);
		//continue;
		
		obj = cJSON_Parse(buf);
		if (!obj)
			return;
		if (this_is_host_heartbeat(obj)) {	// 来之homeserver的主机心跳
			heartbeat_ack();				// 回复主机心跳包
			cJSON_Delete(obj);
			continue;
		}
		else if (this_is_write_packet(obj)) {
			if (!parse_write_packet(obj, &packet_head)) {
				CUR;
				cJSON_Delete(obj);
				continue;
			}
			send_write_packet(&packet_head);
		}
		else {
			cJSON_Delete(obj);
			continue;
		}
		
		free_write_packet_list(&packet_head);	// 释放包链表
		usleep(IRcode_SEND_CYCLE);
	}
}



#include <conf.h>
#include <beacon.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <itach.h>
#include <list.h>
#include <IRdev.h>
#include <itach_cmd.h>
#include <comm_gateway.h>
#include <cJSON.h>

bool scan_ending_flag;


static LIST_HEAD(itach_tmp_head);
//static pthread_mutex_t itachTmpList_mutex = PTHREAD_MUTEX_INITIALIZER;
static void  inline print_itach(itach_t *itach)
{
	DBG("%s %s\n", itach->uuid, itach->ip);
}
static void  inline print_itach_list(list_t *head)
{
	list_t *pos;
	itach_t *itach;
	list_for_each(pos, head) {
		itach = container_of(pos, itach_t, list);
		print_itach(itach);
	}
}

itach_t *get_itach(const char *uuid)
{
	list_t *pos;
	itach_t *itach;
	CUR;
	pthread_mutex_lock(&g_itachs_mutex);
	list_for_each(pos, &g_itach_head) {
		itach = container_of(pos, itach_t, list);
		if (!strcmp(itach->uuid, uuid)) {
			goto out;
		}
	}
	itach = NULL;
out:
	pthread_mutex_unlock(&g_itachs_mutex);
	CUR;
	return itach;
}

static char  *get_ip(list_t *head, const char *uuid)
{
	list_t *pos;
	itach_t *itach;
	list_for_each(pos, head) {
		itach = container_of(pos, itach_t, list);
		if (!strcmp(itach->uuid, uuid))
			return itach->ip;
	}
	return NULL;
}
static itach_t *itach_moudule_existed(list_t *itach_head, const char *uuid)
{
	list_t *pos;
	itach_t *itach;
	list_for_each(pos, itach_head) {
		itach = container_of(pos, itach_t, list);
		if (!strcmp(itach->uuid, uuid)) {
			return itach;
		}
	}
	return NULL;
}

static void free_itach_list(list_t *head)
{
	list_t *pos, *p;
	itach_t *itach;
	list_for_each_prev_safe(pos, p, head){
		itach = container_of(pos, itach_t, list);
		list_del(pos);		// 从节点中删除
		free(itach);		// 释放空间
	}
	INIT_LIST_HEAD(head);
}

static itach_t *insert_new_itach(list_t *itach_head, const char *uuid, const char *ip)
{
	itach_t *itach = malloc(sizeof(itach_t));
	if (!itach) {
		DBG("ERROR: malloc itach\n");
		exit(0);
	}
	memset(itach, 0, sizeof(itach_t));
	strcpy(itach->ip, ip);
	strcpy(itach->uuid, uuid);
	
	list_add_tail(&(itach->list), itach_head);	
	return itach;
}


/////////////////////////////////////////////////////////////////////////////////////////
static void updata_itach_ip(itach_t *itach, const char *ip)
{
	if (strcmp(itach->ip, ip)) 		{// itach模块ip改变
		memset(itach->ip, 0, 16);
		strcpy(itach->ip, ip);
	}
}

static void check_new_itach()
{
	list_t *pos;
	itach_t *itach, *new_itach;
	
	/* 检查itach模块链表:
	 *	1.是否有新模块被检测到，如果有就建立新连接
	 *	2.已经存在的itach模块ip是否改变, 如果改变重连。
	 */
	pthread_mutex_lock(&g_itachs_mutex);
	list_for_each(pos, &itach_tmp_head)
	{
		itach = container_of(pos, itach_t, list);
		// 新模块
		if (!itach_moudule_existed(&g_itach_head, itach->uuid)) {
			new_itach = insert_new_itach(&g_itach_head, itach->uuid, itach->ip);
			connect_to_itach(new_itach);
			DBG("new itach: %s %s\n", itach->uuid, itach->ip);
		}
	}	// 检查完成	
	pthread_mutex_unlock(&g_itachs_mutex);
}




// 检查itach模块掉线
static void  check_itach_ip_and_lost()
{
	list_t *pos;
	itach_t *itach;
	char *ip;
	
	pthread_mutex_lock(&g_itachs_mutex);
	list_for_each(pos, &g_itach_head) {
		itach = container_of(pos, itach_t, list);
		// itach模块掉线
		if (!itach_moudule_existed(&itach_tmp_head, itach->uuid)){
			DBG("itach moudule: %s %s lost..\n", itach->uuid, itach->ip);

			// 1.上报itach模块掉线(上报itach模块上连接的所有设备)
			upload_itach_lost(itach);
			DBG("upload itach lost ok...\n");
			
			// 2.从g_itach_head链表中删除这个节点
			list_del(&itach->list);
		
			// 3.释放空间
			free(itach);
		} 
		else {
			ip = get_ip(&itach_tmp_head, itach->uuid);
			if (!ip) {
				DBG("ip is empty\n");
				exit(0);
			}
			if (strcmp(itach->ip, ip)) {
				memset(itach->ip, 0, strlen(itach->ip));
				strcpy(itach->ip, ip);
				reconnection_itach(itach);
				DBG("%s ip changed: %s, already reconnection...\n", itach->uuid, itach->ip);
			}
		}
	}
	pthread_mutex_unlock(&g_itachs_mutex);
}

// 上位机会定时读取这个文件
void record_itachs()
{
	list_t *pos;
	itach_t *itach;
	cJSON *obj;
	cJSON *arry;
	cJSON *item;
	char *data;
	if (!file_existed(ITACHS_CONFIG_PATH)) 
		create_file(ITACHS_CONFIG_PATH);
	
	obj = cJSON_CreateObject();
	arry = cJSON_CreateArray();
	cJSON_AddItemToObject(obj, "itachs_config", arry);

	pthread_mutex_lock(&g_itachs_mutex);
	list_for_each(pos, &g_itach_head) {
		itach = container_of(pos, itach_t, list);
		item = cJSON_CreateObject();
		cJSON_AddStringToObject(item, "uuid", itach->uuid);
		cJSON_AddStringToObject(item, "ip", itach->ip);
		cJSON_AddItemToArray(arry, item);
	}
	pthread_mutex_unlock(&g_itachs_mutex);
	clear_file(ITACHS_CONFIG_PATH);
	write_file(ITACHS_CONFIG_PATH, data = cJSON_Print(obj));
	cJSON_Delete(obj);
	free(data);
}

void sigalarm(int sig)
{
	DBG("canning ending............................................................\n");
	check_itach_ip_and_lost();
	check_new_itach();
	print_itach_list(&itach_tmp_head);
	scan_ending_flag = true;
}

/* 1.以20s为一个周期, 记录一次存在的itach模块信息。
   2.每一个周期后，将当前记录的设备链表与上一次记录的表进行比对
   		1.检测到itach模块删减要上报
   		2.已经断开连接的模块，要关闭套接字， 新连接的模块要进行TCP连接
   		2.要对维护的itach链表进行修改
*/
static void *work_scanning(void *args)
{
	char beacon[200];
	char ip[16];
	char uuid[25];
	int udpfd;
	itach_t *itach;
	//sigset_t set;
	
	pthread_detach(pthread_self());
	INIT_LIST_HEAD(&g_itach_head);
	INIT_LIST_HEAD(&itach_tmp_head);
	scan_ending_flag = false;
	
	signal(SIGALRM, sigalarm);
	alarm(ITACH_SCANNING_CYCLE);	// 15s后给进程发出SIGALRM信号
	
	if (-1 == (udpfd = beacon_init())) 
		goto err;
	while(1)
	{	
		if (scan_ending_flag) {
			free_itach_list(&itach_tmp_head);
			alarm(ITACH_SCANNING_CYCLE);
			scan_ending_flag = false;
			record_itachs();	// 上位机会定时读取这个文件
		}
		memset(beacon, 0, sizeof(beacon));
		recvfrom(udpfd, beacon, sizeof(beacon), 0, NULL, NULL);
		get_uuid_from_beacon(beacon, uuid);
		get_ip_from_beacon(beacon, ip);
		
		itach = itach_moudule_existed(&itach_tmp_head, uuid);
		if (itach) {
			updata_itach_ip(itach, ip);
		} else {
			itach = insert_new_itach(&itach_tmp_head, uuid, ip);
			//DBG("new itach: %s %s\n", itach->uuid, itach->ip);
		}
		// 线程取消阻塞所有信号
	}
	return NULL;
err:
	exit(-1);
}

void scanning_itachs()
{
	pthread_t uid;
	// 每20s将扫描到的itach设备放入临时链表
	pthread_create(&uid, NULL, work_scanning, NULL);
}



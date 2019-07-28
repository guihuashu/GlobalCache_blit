#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <IRdev.h>
#include <cJSON.h>
#include <string.h>
#include <pthread.h>
#include <list.h>
#include <debug.h>
#include <status_file.h>

static char *read_json_file(const char *filename)
{
	FILE *f;long len;char *data;
	int ret;
	if (-1 == access(filename, F_OK))  {
		DBG("file no exist\n");
		goto err;
	}
	f=fopen(filename,"rb");			// 以可读二进制方式打开文件
	if (!f) 
		goto err;
	fseek(f,0,SEEK_END);				// 将文件的偏移值设置到文件末尾
	len=ftell(f);						// 确定文件当前偏移位置
	fseek(f,0,SEEK_SET);				// 将文件的偏移值设置到文件开始位置
	data=(char*)malloc(len+1);			// 分配 len +1
	ret = fread(data,1,len,f);			// 从f中读取数据到data, 每次读1个字节, 共读len次
	fclose(f);							// 关闭文件
	return data;
err:
	DBG("ERR: read_json_file!\n");
	exit(0);
}



static void get_IRdevs_from_jsonObj(cJSON *obj)
{
	int dev_cnt; 
	int IRcodeCnt;
	int  i,j;
	IRdev_code_t *code;
	//int k;
	
	cJSON *dev_arry = 	cJSON_GetObjectItem(obj,"objects");
	dev_cnt = cJSON_GetArraySize(dev_arry);
	if (status_file_is_empty())
		init_status_file();
	for ( i= 0; i < dev_cnt; ++i) 
	{
		// 提取json对象中的设备数据
		cJSON *IRdev 	= cJSON_GetArrayItem(dev_arry, i);
		char *name 		= cJSON_GetObjectItem(IRdev, "name")->valuestring;
		char *uuid 		= cJSON_GetObjectItem(IRdev, "uuid")->valuestring;
		//int ChannelNo 	= cJSON_GetObjectItem(IRdev, "ChannelNo")->valueint;
		
		cJSON *function_arry =	cJSON_GetObjectItem(IRdev, "function");
		cJSON *function_arry0 = cJSON_GetArrayItem(function_arry, 0);
		char  *sid  	= cJSON_GetObjectItem(function_arry0, "sid")->valuestring;
		cJSON *attr 	= cJSON_GetObjectItem(function_arry0, "attr");
		
		char *type  	= cJSON_GetObjectItem(attr, "type")->valuestring;
		cJSON *IRCode_arry 	= cJSON_GetObjectItem(attr, "IRCode");
		IRcodeCnt		= cJSON_GetArraySize(IRCode_arry);
		
		IRdev_t *dev = malloc(sizeof(IRdev_t));
		INIT_LIST_HEAD(&dev->IRcode_head);
		
		strcpy(dev->name, name);
		strcpy(dev->uuid, uuid);
		//dev->ChannelNo = ChannelNo;
		dev->IRcodeCnt = IRcodeCnt;
		strcpy(dev->sid, sid);
		strcpy(dev->type, type);
		for ( j = 0; j < IRcodeCnt; j++) {
			char *IRcode = cJSON_GetArrayItem(IRCode_arry, j)->valuestring;
			code = malloc(sizeof(IRdev_code_t));
			code->IRcode = malloc(strlen(IRcode) +1);
			code->No = j;
			memset(code->IRcode, 0, strlen(IRcode) +1);
			strcpy(code->IRcode, IRcode);
			list_add_tail(&code->list, &dev->IRcode_head);
		}
		dev->status_value = get_status_form_file(dev->sid);
		
		// 新IR设备插入链表
		list_add_tail(&dev->list, &g_IRdevs_head);
		
#if 0
		// 记录设备信息
		IRdev_t *dev = malloc_IRdev(IRcodeCnt);
		for ( j = 0; j < IRcodeCnt; j++) {
			char *IRcode = cJSON_GetArrayItem(IRCode_arry, j)->valuestring;
			dev->IRcode_arry[j] = malloc(strlen(IRcode) +1);
			if (!dev->IRcode_arry[j]) {
				DBG("malloc dev->IRcode_arry[j] err\n");
				exit(0);
			}
			bzero(dev->IRcode_arry[j], strlen(IRcode) +1);
			strcpy(dev->IRcode_arry[j], IRcode);
		}
		
#endif
	}
}

void read_IRdevs_from_jsonfile(const char *path)
{
	const char *json_text = read_json_file(path);
	
	cJSON *obj = cJSON_Parse(json_text);
	if (!obj) 
		goto err;
	get_IRdevs_from_jsonObj(obj);
	
	cJSON_Delete(obj);
//	free(json_text);
	return;
	
err:
	DBG("ERR: %s fmt err!\n", path);
	exit(0);
}

IRdev_t *get_IRdev(const char *sid)
{
	list_t *pos;
	IRdev_t *IRdev;
	
	pthread_mutex_lock(&g_IRdevs_mutex);
	list_for_each(pos, &g_IRdevs_head) {
		IRdev = container_of(pos, IRdev_t, list);
		if (!strcmp(IRdev->sid, sid)) {
			goto out;
		}
	}
	IRdev = NULL;
out:
	pthread_mutex_unlock(&g_IRdevs_mutex);
	return IRdev;
}

void print_IRcodes(IRdev_t *IRdev)
{
	list_t *pos;
	IRdev_code_t *code;
	list_for_each(pos, &IRdev->IRcode_head) {
		code = container_of(pos, IRdev_code_t, list);
		printf("No = %d\n", code->No);
		printf("IRcode:\n%s\n", code->IRcode);
	}
}

void print_IRdev(IRdev_t *IRdev)
{
	DBG("name: %s\n", IRdev->name);
	DBG("type: %s\n", IRdev->type);
	DBG("uuid: %s\n", IRdev->uuid);
	DBG("sid: %s\n", IRdev->sid);
	DBG("IRcodeCnt: %d\n", IRdev->IRcodeCnt);
	//DBG("ChannelNo: %d\n", IRdev->ChannelNo);
	print_IRcodes(IRdev);
}
void print_IRdevs_list(list_t *list)
{
	list_t *pos;
	IRdev_t *IRdev;
	
	pthread_mutex_lock(&g_IRdevs_mutex);
	DBG("g_IRdevs_head print.......\n\n");
	list_for_each(pos, &g_IRdevs_head) {
		IRdev = container_of(pos, IRdev_t, list);
		DBG("-----------------------\n\n");
		print_IRdev(IRdev);
	}
	pthread_mutex_unlock(&g_IRdevs_mutex);
	
}

char *find_IRCode(IRdev_t *dev, int No)
{
	list_t *pos;
	IRdev_code_t *code;
	list_for_each(pos, &dev->IRcode_head) {
		code = container_of(pos, IRdev_code_t, list);
		if (code->No == No) {
			printf("No = %d\n", code->No);
			printf("IRcode:\n%s\n", code->IRcode);
			return code->IRcode;
		}
	}
	return NULL;
}


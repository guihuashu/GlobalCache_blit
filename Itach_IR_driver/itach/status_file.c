#include <debug.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <public_opr.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <cJSON.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <conf.h>

/*
	1.如果状态文件为空, 就设置状所有设备初始状态为0;
	2.上报设备状态时, 将设备状态记录到文件中
	3.程序重启后,从状态文件中读取设备状态
*/

bool status_file_is_empty()
{
	struct stat stat_;
	
	if (!file_existed(STATUS_FILE_PATH)) {
		DBG("ERROR: %s not exist\n", STATUS_FILE_PATH);
		exit(0);
	}
	memset(&stat_, 0, sizeof(stat_));
	stat(STATUS_FILE_PATH, &stat_);
	return (0 == stat_.st_size);
}
static void clean_status_file()
{
	truncate(STATUS_FILE_PATH, 0);
}

static cJSON *read_status_file()
{
	char data[1024];
	cJSON *obj ;
	int fd = open(STATUS_FILE_PATH, O_RDWR);	
	if (fd <= 0) {
		DBG("ERROR: open %s \n", STATUS_FILE_PATH);
		exit(0);
	}
	memset(data, 0, sizeof(data));
	if (-1 == read(fd, data, sizeof(data)))
		goto err;
	
	printf("------\n%s\n", data);
	if (!(obj = cJSON_Parse(data)))
		goto err;
	close(fd);
	
	return 	obj;
err:
	close(fd);
	DBG("ERROR: read_status_file\n");
	exit(0);
}

static void overwrite_status_file(cJSON *obj)
{
	char *data = cJSON_Print(obj);
	int fd = open(STATUS_FILE_PATH, O_RDWR);	
	if (fd <= 0) {
		DBG("ERROR: open %s \n", STATUS_FILE_PATH);
		exit(0);
	}
	write(fd, data, strlen(data));
	close(fd);
	free(data);
}
void print_status_file()
{
	cJSON *obj = read_status_file();
	char *data = cJSON_Print(obj);
	printf("-----------------------------------------------\n%s\n", data);
	free(data);
	cJSON_Delete(obj);
}




static void add_status(const char *sid, int status_value)
{
	cJSON *obj = read_status_file();
	cJSON_AddNumberToObject(obj, sid, status_value);
	overwrite_status_file(obj);
	cJSON_Delete(obj);
}
static void modify_status(const char *sid, int status_value)
{
	cJSON *obj = read_status_file();
	
	cJSON_DeleteItemFromObject(obj, sid);
	cJSON_AddNumberToObject(obj, sid, status_value);
		
	overwrite_status_file(obj);
	cJSON_Delete(obj);
}
void init_status_file()
{
	clean_status_file();
	cJSON *obj = cJSON_CreateObject();
	cJSON_AddNumberToObject(obj, "test_sid", 00);
	overwrite_status_file(obj);
	cJSON_Delete(obj);
}

void record_status(const char *sid, const int status_value)
{
	cJSON *sid_obj;
	cJSON *obj;
	if (status_file_is_empty()){
		init_status_file();
	}

	obj = read_status_file();
	sid_obj = cJSON_GetObjectItem(obj, sid);
	
	if (!sid_obj)	// 文件中没有状态
		add_status(sid, status_value);
	else  
		modify_status(sid, status_value);
	cJSON_Delete(obj);
}

int get_status_form_file(const char *sid)
{
	cJSON *obj;
	cJSON *sid_obj;
	int ret;

	if (status_file_is_empty()){
		init_status_file();
	}
	obj = read_status_file();
	sid_obj = cJSON_GetObjectItem(obj, sid);
	
	if (!sid_obj) {
		ret = 0;
		add_status(sid, 0);	// 默认状态为0
	}
	else {
		ret =  sid_obj->valueint;
	}
	cJSON_Delete(obj);
	return ret;
	
}




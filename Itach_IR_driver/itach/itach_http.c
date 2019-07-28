#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

#include <cJSON.h>
#include <IRdev.h>
#include <itach.h>
#include <itach_http.h>
#include <itach_cmd.h>
#include <debug.h>
static bool sub_status2;
static bool sub_status3;
static bool sub_status4;
static int inline calc_body_size(const char *IRcode)
{
    int size = strlen("frequency");
    size +=  strlen(":");
    size +=  strlen("38095");
    size +=  strlen(",\n\r");
    size +=  4;                 /* 两对冒号 */

    size +=  strlen("preamble");
    size +=  strlen(":");
    size +=  strlen("320,160");
    size +=  strlen(",\n\r");
    size +=  4;                 /* 两对冒号 */

    size += strlen("irCode");
    size +=  strlen(":");
    size +=  strlen(IRcode);
    size +=  strlen(",\n\r");
    size +=  4;                 /* 两对冒号 */

    size += strlen("repeat");
    size +=  strlen(":");
    size +=  strlen("1");
    size +=  4;                 /* 两对冒号 */

	return size;
}

/* # POST /api/v1/irports/{IR_port_number}/sendir
	+ Response 200
	    + Headers
	    
	            Connection:close
	            Content-Type:application/json
	            Cache-Control:no-cache

	    + Body
*/

/* 正常回复的数据:
	4
	{
	        "frequency":    "38095",
	        "preamble":     "320,160",
	        "irCode":       "20,60,20,60,20,20,20,20,20,20,20,20,20,20,20,20,20,60,20,60,20,60,20,20,20,60,20,20,20,20,20,20,20,873",
	        "repeat":       "1"
	}
	0
	upload completely sent off: 182 out of 182 bytes

	1
	HTTP/1.1 200 OK

	1
	Connection: close

	1
	Content-Type: application/json

	1
	Cache-Control: no-cache

	1


	3

	0
	Closing connection 0
*/
static void post_backcall(CURL *handle, curl_infotype type, char *data, size_t size,  void *userp)
{

	// 任何数据交互(发送,接受, 协议头...)都会回调调用该函数, 这个函数会被多次调用
	if (type != CURLINFO_HEADER_IN)
		return;

	DBG("%s\n", data);
	
	printf("%s\n", data);	// Connection: close
	if (!strncmp(data, "Connection: close", size-2)) {
		sub_status2 = true;
	}
	else if (!strncmp(data, "Content-Type: application/json", size-2)) {
		sub_status3 = true;
	}
	else if (!strncmp(data, "Cache-Control: no-cache", size-2)){
		sub_status4 = true;
	}
	
}

static void itach_post(const char *ip, const int port, const char *IRcode)	// port: 红外码发送的通道号
{
	CUR;
	struct curl_slist *headers=NULL;
	char url[100];
	char Length[50];
	cJSON *obj;
	char *body;
	int body_size;
	
	memset(url, 0,  sizeof(url));
	memset(Length, 0,  sizeof(Length));
	
	// 开始设置
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *curl = curl_easy_init();

	// 设置头
	body_size = calc_body_size(IRcode);
	sprintf(Length, "Content-Length:%d", body_size);
	printf("Length:%s\n", Length);
	curl_slist_append(headers, Length);
	curl_slist_append(headers, "Content-Type: application/json");

#if 0	// 这儿处理有问题, 因为发送不同的红外码可能有不同的 "frequency" "preamble" "repeat"
	// 设置body
	obj = cJSON_CreateObject();
	cJSON_AddStringToObject(obj, "frequency", "38095");
	cJSON_AddStringToObject(obj, "preamble", "320,160");
	cJSON_AddStringToObject(obj, "irCode", IRcode);
	cJSON_AddStringToObject(obj, "repeat", "1");
	body = cJSON_Print(obj);
	printf("body:\n%s\n", body);
#endif

	// 设置url
	sprintf(url, "http://%s/api/v1/irports/%d/sendir", ip, port);
	curl_easy_setopt(curl, CURLOPT_URL, "http://172.16.24.251/api/v1/irports/2/sendir");
	printf("url:%s\n", url);
	
	
	// 设置回调函数
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, post_backcall);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	// 设置http头
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// 设置body
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);

	// 开始post
	curl_easy_perform(curl);
	
	curl_slist_free_all(headers);
	free(body);
	cJSON_Delete(obj);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void http_send_IRcode(struct write_packet *packet)
{
	IRdev_t *IRdev = get_IRdev(packet->sid);
	if (!IRdev)
		goto err;

	if (packet->value <= 0)
		goto err;
	const char *IRcode = IRdev->IRcode_arry[packet->value -1];	//从第0项开始存放红外码
	//printf("IRcode: %s\n", IRcode);
	
	itach_t *itach = get_itach(IRdev->uuid);
	
	if (!itach) 
		goto err;
	
	sub_status2 = false;
	sub_status3 = false;
	sub_status4 = false;
	
	CUR;
	itach_post(itach->ip, IRdev->ChannelNo, IRcode);
	
	return;
err:
	DBG("ERROR: http_send_IRcode\n");
	exit(0);
}


bool IRcode_http_send_ok()
{
	return (sub_status2 && sub_status3 && sub_status4);
}


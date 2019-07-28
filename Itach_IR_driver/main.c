#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include <itach.h>
#include <conf.h>
#include <cJSON.h>
#include <IRdev.h>
#include <signal.h>
#include <unistd.h>
#include <list.h>
#include <debug.h>
#include <comm_gateway.h>

// itach设备链表
LIST_HEAD(g_itach_head);
LIST_HEAD(g_IRdevs_head);
pthread_mutex_t g_itachs_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_IRdevs_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	read_IRdevs_from_jsonfile(JSON_FILE_PATH); // 从json文件中读IR设备
	
	print_IRdevs_list(&g_IRdevs_head);
	scanning_itachs();			// 开线程, 扫描itach模块
	CUR;
	/* 确保接受高级网关控制时, itach模块的ip已经被记录 */
	sleep(ITACH_SCANNING_CYCLE);

	CUR;
	connect_to_gateway();			// 连接到高级网关
	//sending_hostheart_packet();	// 开线程,发送心跳包
	CUR;
	upload_init_IRdevs_status();// 上报初始设备状态
	CUR;
	comm_gw();					// 接受高级网关控制
	return 0;
}

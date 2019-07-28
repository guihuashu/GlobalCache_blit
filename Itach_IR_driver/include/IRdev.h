
#ifndef IR_DEV_H
#define IR_DEV_H

#include <list.h>

#define IRCODE_LEN 300
#define UUID_LEN 25;


extern list_t g_IRdevs_head;
extern pthread_mutex_t g_IRdevs_mutex;

/* 按照HDL高级网关目标类型V2.7.pdf的规定:
		1.每一个功能对应唯一的一个红外码
		2.从1开始由小到大连续定义设备家电的每一个功能
		3.json文件将严格按照此文件给出的功能顺序,来存放红外码
*/
typedef struct IRdev_code{
	int No;			// 红外码在json文件中的数组序号, 从0开始	
	char *IRcode;
	list_t list;
}IRdev_code_t;

typedef struct IRdev{
	char name[50];
	char type[50];
	char uuid[25];
	char  sid[65];			/* 64个字节 */
	int  IRcodeCnt;			/* 拥有的红外码个数: 从1开始 */
	list_t IRcode_head;		/* 红外码数组 */
	list_t list;
	int status_value;
}IRdev_t;

IRdev_t *get_IRdev(const char *sid);
void read_IRdevs_from_jsonfile(const char *path);
void print_IRcodes(IRdev_t *IRdev);
void print_IRdevs_list(list_t *list);
void print_IRdev(IRdev_t *IRdev);
char *find_IRCode(IRdev_t *dev, int No);

#endif //IR_DEV_H


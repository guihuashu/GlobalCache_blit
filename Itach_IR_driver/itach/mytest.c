#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>
#include "cJSON.h"

char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":		 \"rect\", \n\"width\": 	 1920, \n\"height\":	 1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";	
char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
char text3[]="[\n	 [0, -1, 0],\n	  [1, 0, 0],\n	  [0, 0, 1]\n	]\n";
char text4[]="{\n		\"Image\": {\n			\"Width\":	800,\n			\"Height\": 600,\n			\"Title\":	\"View from 15th Floor\",\n 		\"Thumbnail\": {\n				\"Url\":	\"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":	\"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n }";
char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":	37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":	\"\",\n  \"City\":		\"SAN FRANCISCO\",\n	 \"State\": 	\"CA\",\n	 \"Zip\":		\"94107\",\n	 \"Country\":	\"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":	37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":	\"\",\n  \"City\":		\"SUNNYVALE\",\n	 \"State\": 	\"CA\",\n	 \"Zip\":		\"94085\",\n	 \"Country\":	\"US\"\n	 }\n	 ]";

#define printStr(x) printf("%s\n", x) 
#define printInt(x) printf("%d\n", x) 
#define printBool(x) \
	if (x)	\
		printf("true"); \
	else \
		printf("false")

void print_obj(cJSON *obj)
{
	if (obj) {
		char *out = cJSON_Print(obj);
		printStr(out);
		cJSON_Delete(obj);
		free(out);
	}
}

void test_cJSON_Print(const char *text)
{
	cJSON *obj = cJSON_Parse(text);
	if (obj) {
		char *out = cJSON_Print(obj);
		printStr(out);
		cJSON_Delete(obj);
		free(out);
	}
}

char *read_file(const char *filename)
{
	FILE *f;long len;char *data;
	
	f=fopen(filename,"rb");			// 以可读二进制方式打开文件
	if (!f)
		return NULL;
	fseek(f,0,SEEK_END);			// 将文件的偏移值设置到文件末尾
	len=ftell(f);					// 确定文件当前偏移位置
	fseek(f,0,SEEK_SET);			// 将文件的偏移值设置到文件开始位置
	data=(char*)malloc(len+1);		// 分配 len +1
	fread(data,1,len,f);			// 从f中读取数据到data, 每次读1个字节, 共读len次
	fclose(f);						// 关闭文件
	return data;
}


void test_read_file(const char *filename)
{
	char *text = read_file(filename);
	test_cJSON_Print(text);	
	free(text);
}

/*
	{
		"name":	"Jack (\"Bee\") Nimble",
		"format":	{
			"type":	"rect",
			"width":	1920,
			"height":	1080,
			"interlace":	false,
			"frame rate":	24,
			"_NULL_":	null,
			"arry":[{"k1":11}, {"k2":true}]
			"arry2":[1,2,3];
		}
	}
*/
char *construct_json_obj()
{
	// 创建对象
	cJSON *obj = cJSON_CreateObject();
	
	cJSON *fmt, *arry, *arry2;

	// 添加item, 键值为字符串
	cJSON_AddStringToObject(obj, "name", "Jack (\"Bee\") Nimble");
	
	// 添加item, 键值为json对象
	cJSON_AddItemToObject(obj, "format", fmt = cJSON_CreateObject());
	
	cJSON_AddStringToObject(fmt, "type", "rect");

	// 添加item, 键值为数字
	cJSON_AddNumberToObject(fmt, "width", 1920);
	cJSON_AddNumberToObject(fmt, "height", 1080);

	// 添加item, 键值为bool
	cJSON_AddBoolToObject(fmt, "interlace", false);
	
	cJSON_AddNumberToObject(fmt, "frame rate", 24);


	// 添加item, 键值为null
	cJSON_AddNullToObject(fmt, "_NULL_");
	
	// 数组中插入json对象
	arry = cJSON_CreateArray();
	cJSON *item1 = cJSON_CreateObject();
	cJSON *item2 = cJSON_CreateObject();
	cJSON_AddNumberToObject(item1, "k1", 11);
	cJSON_AddNumberToObject(item2, "k2", 22);
	cJSON_AddItemToArray(arry, item1);
	cJSON_AddItemToArray(arry, item2);
	
	// 数组中插入值
	arry2 = cJSON_CreateArray();
	cJSON_AddNumberToObject(arry2, "", 1);
	cJSON_AddNumberToObject(arry2, "", 2);
	cJSON_AddNumberToObject(arry2, "", 3);

	// json对象中插入数组
	cJSON_AddItemToObject(obj, "arry", arry);
	cJSON_AddItemToObject(obj, "arry2", arry2);
	
	//print_obj(obj);
	char *out = cJSON_Print(obj);
	
	cJSON_Delete(obj);
	return out;
}
/*
	{
		"name":	"Jack (\"Bee\") Nimble",
		"format":	{
			"type":	"rect",
			"width":	1920,
			"height":	1080,
			"interlace":	false,
			"frame rate":	24,
			"_NULL_":	null,
			"arry":[{"k1":11}, {"k2":true}]
			"arry2":[1,2,3];
		}
	}
*/
int main (int argc, char **argv)
{
	// cJSON_Parse
	//test_cJSON_Print(text1);

	// 从文件中读取json
	//test_read_file("./tests/test1");

	// 构造json对象
	char *text = construct_json_obj();
	//printStr(text);
	
	// 解析json对象
	cJSON *obj =  cJSON_Parse(text);

	// cJSON对象中,键值为valuestring
	char *valuestring = cJSON_GetObjectItem(obj, "name")->valuestring;
	printStr(valuestring);

	cJSON *fmt = cJSON_GetObjectItem(obj, "format");
	valuestring = cJSON_GetObjectItem(fmt, "type")->valuestring;
	printStr(valuestring);

	// cJSON对象中,键值为valueint
	int valueInt = cJSON_GetObjectItem(fmt, "width")->valueint;
	printInt(valueInt);

	valueInt = cJSON_GetObjectItem(fmt, "height")->valueint;
	printInt(valueInt);

	
	// cJSON对象中,键值为bool(0/1)
	int valuebool = cJSON_GetObjectItem(fmt, "interlace")->valueint;	/*注意: 0 或者 1*/ 
	printInt(valuebool);

	// cJSON对象中,键值为null(NULL)
	if (NULL == cJSON_GetObjectItem(fmt, "_NULL_")->valuestring)		/*注意: null是空指针NULL */
		printf("valuestring is null\n");

	
	// cJSON对象中,键值为数组
	printStr("------------------------------------------------");
	cJSON *arry =  cJSON_GetObjectItem(obj, "arry");
	cJSON *arry2 =  cJSON_GetObjectItem(obj, "arry2");

	// 获取数组中项目个数
	int len = cJSON_GetArraySize(arry);
	printInt(len);

	// 数组对象, 键值为cJSON对象
	cJSON *subItem = cJSON_GetArrayItem(arry, 0);
	// cJSON对象中,键值为valueint
	valueInt = cJSON_GetObjectItem(subItem, "k1")->valueint;
	printInt(valueInt);

	// 数组对象, 键值为cJSON对象
	subItem = cJSON_GetArrayItem(arry, 1);
	// cJSON对象中,键值为valueint
	valueInt = cJSON_GetObjectItem(subItem, "k2")->valueint;
	printInt(valueInt);

	printStr("------------------------------------------------");
	// arry2
	len = cJSON_GetArraySize(arry2);
	printInt(len);

	// 数组对象中的值
	valueInt = cJSON_GetArrayItem(arry2, 0)->valueint;
	printInt(valueInt);

	valueInt = cJSON_GetArrayItem(arry2, 1)->valueint;
	printInt(valueInt);
	
	valueInt = cJSON_GetArrayItem(arry2, 2)->valueint;
	printInt(valueInt);
	cJSON_Delete(obj);
	free(text);
	
	//cJSON_Delete(obj);
	return 0;
}






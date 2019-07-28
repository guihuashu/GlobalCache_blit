
#include <conf.h>
#include <string.h>
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <debug.h>
#include <sys/stat.h>
#include <fcntl.h>

bool list_is_empty(list_t *list)
{
	return (list->prev == list->next);
}
void rm_file(const char *path)
{
	char cmd[50];
	sprintf(cmd, "rm -rf %s", path);
	system(cmd);
}

bool file_existed(const char *path)
{
	return !access(path, F_OK);
}

void create_file(char *path)
{
	char cmd[100];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "touch %s", ITACHS_CONFIG_PATH);
	system(cmd);
}
void clear_file(char *path)
{
	if (!file_existed(path)) {
		DBG("ERROR: clear_file, file not exist\n");
		exit(0);
	}
	truncate(path, 0);
}
void write_file(char *path, char *data)
{
	int fd = open(path, O_RDWR);	
	if (fd <= 0) {
		DBG("ERROR: open %s \n", STATUS_FILE_PATH);
		exit(0);
	}
	write(fd, data, strlen(data));
	close(fd);
}



#ifndef PUBLIC_OPR_H
#define PUBLIC_OPR_H

#include <stdbool.h>
#include <list.h>


typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;

bool list_is_empty(list_t *list);
void rm_file(const char *path);
bool file_existed(const char *path);
void create_file(char *path);
void clear_file(char *path);
void write_file(char *path, char *data);

#endif	// PUBLIC_OPR_H


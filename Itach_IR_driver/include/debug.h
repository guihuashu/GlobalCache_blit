#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#define DBG(fmt, args...)  printf(fmt, ##args)
//#define DBG(fmt, ...)
#define CUR printf("%s, %d\n", __FILE__, __LINE__);
//#define CUR //printf("%s, %d\n", __FILE__, __LINE__);

#endif //DEBUG_H

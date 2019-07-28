
#ifndef _BEACON_H
#define _BEACON_H

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <public_opr.h>
#include <pthread.h>
#include <itach.h>
#include <IRdev.h>
#include <signal.h>
#include <unistd.h>
#include <list.h>
#include <debug.h>


void get_uuid_from_beacon(char *beacon, char *uuid);
void get_ip_from_beacon(char *beacon, char *ip);
int beacon_init();

extern list_t g_itach_head;
extern pthread_mutex_t g_itachs_mutex;

#endif //_BEACON_H

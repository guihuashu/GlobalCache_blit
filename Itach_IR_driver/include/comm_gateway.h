
#ifndef COMM_GATEWAY_H
#define COMM_GATEWAY_H

#include <list.h>
#include <pthread.h>
#include <itach.h>

void connect_to_gateway();
void sending_hostheart_packet();
void upload_itach_lost(itach_t *itach);
void upload_init_IRdevs_status();
void comm_gw();

void comm_gw_recvfrom_init();

typedef struct write_packet {
	char  sid[65];			/* 64个字节 */
	int value;
	list_t list;
}write_packet_t;



#endif	// COMM_GATEWAY_H

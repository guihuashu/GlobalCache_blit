

#ifndef _ITACH_CMD_H
#define _ITACH_CMD_H

#include <itach.h>
#include <list.h>
#include <stdbool.h>
#include <comm_gateway.h>
void connect_to_itach(itach_t *itach);
void reconnection_itach(itach_t *itach);
void control_itachs(list_t *head);
bool tcp_send_IRcode(write_packet_t *packet);

#endif // _ITACH_CMD_H

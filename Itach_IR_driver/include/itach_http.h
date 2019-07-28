#ifndef ITACH_HTTP_H
#define ITACH_HTTP_H

#include <comm_gateway.h>

void http_send_IRcode(struct write_packet *packet);
bool IRcode_send_ok();
#endif	// ITACH_HTTP_H


#ifndef CONF_H
#define CONF_H

#include <signal.h>
#define BEACON_MULTI_ADDR   "239.255.250.250"
#define BEACON_MULTI_PORT   9131
#define JSON_FILE_PATH 			"/mnt/unix/config/device/100F6010.json"

/* unix udp 域套接字
 *	1.从/mnt/unix/path/infrared出读取 (高级网关将通过该"管道"向红外驱动发送数据")
 *  2.写入到/mnt/unix/server (所有驱动通过该"管道"向高级网关发送数据)
 */ 
#define SENDTO_GW_PIPE		"/mnt/unix/server"
#define RECVFROM_GW_PIPE	"/mnt/unix/path/infrared"

#define ITACH_DRIVER_NAME	"infrared"
#define ITACH_DRIVER_CODE	"034B4837"

#define ITACH_SCANNING_CYCLE 15		//21s
#define HEARTBEAT_CYCLE 2			//20s

#define ITACH_TCP_PORT 4998


#define STATUS_FILE_PATH 	"/mnt/unix/config/infrared/IRdevs_status"
#define WAIT_IRcode_SEND_TIME 1000000	// 10ms
#define IRcode_SEND_CYCLE 333333	// 0.3s
#define ITACHS_CONFIG_PATH	"/mnt/unix/config/infrared/itachs_config"


#endif	// CONF_H


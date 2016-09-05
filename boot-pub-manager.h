#ifndef __BOOT-PUB-MANAGER_H__
#define __BOOT-PUB-MANAGER_H__

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define DEVICE  "https:\/\/194.65.138.51"
#define RESOURCE "/api/message/topicdanjoseluis"

#define TARGET "https:\/\/194.65.138.51/api/message/secure_bootstrapping"
#define ACTION "POST"

uint8_t capabilityToken[800];
uint16_t  capabilityTokenLen;


PROCESS_NAME(boot_pub_manager_server);

#endif /* __BOOT-PUB-MANAGER_H__ */



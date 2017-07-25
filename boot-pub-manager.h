#ifndef __BOOT-PUB-MANAGER_H__
#define __BOOT-PUB-MANAGER_H__

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define DEVICE  "https:\/\/194.65.138.51"

#define TARGET "https:\/\/194.65.138.51/api/message/secure_bootstrapping"
#define ACTION "POST"

#define PROCESS_EVENT_SUCCESS           137
#define PROCESS_EVENT_FAILURE           138



uint8_t capabilityToken[800];
uint16_t  capabilityTokenLen;
extern uint8_t more;


PROCESS_NAME(boot_pub_manager_server);


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


static inline
void printf_color (const char* color, const char *message){

	printf("%s",color);
	printf("%s",message);
	printf("%s",RESET);
  printf("\n");
}



#endif /* __BOOT-PUB-MANAGER_H__ */



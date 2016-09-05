#ifndef __PUBLICATION_CLIENT_H__
#define __PUBLICATION_CLIENT_H__

#include "er-coap-engine.h"

/* definitions */
#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT+1)
#define REMOTE_PORT     UIP_HTONS(20220)

#define TOGGLE_INTERVAL 10

extern uint8_t capabilityToken[800];
extern uint16_t  capabilityTokenLen;

PROCESS_NAME(publication_client);

#endif /* __PUBLICATION_CLIENT_H__ */

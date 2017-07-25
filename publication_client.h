#ifndef __PUBLICATION_CLIENT_H__
#define __PUBLICATION_CLIENT_H__

#include "er-coap-engine.h"

/* definitions */

#define TOGGLE_INTERVAL 10

extern uint8_t capabilityToken[800];
extern uint16_t  capabilityTokenLen;

uint8_t more;


PROCESS_NAME(publication_client);

#endif /* __PUBLICATION_CLIENT_H__ */

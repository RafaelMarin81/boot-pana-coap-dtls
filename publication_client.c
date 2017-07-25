#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "er-coap-engine.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#include "rest-engine.h"

#include "publication_client.h"
#include "boot-pub-manager.h"


#include "node-id.h"

PROCESS(publication_client, "Publishing Information");

#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x1)

uip_ipaddr_t server_ipaddr;
static struct etimer et;
static int sent = 0;

char endp[64];

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;
  const char *str=NULL;

  int len = coap_get_header_location_path(response, &str);
  endp[0] = '/';
  memcpy(endp+1, str, len);
  endp[len] = '\0';
  //printf("Location-path: %s\n", endp);

  len = coap_get_payload(response, &chunk);
  //PRINT("|%.*s", len, (char *)chunk);

}

static char publication[900];
static coap_packet_t request[1];/* This way the packet can be treated as pointer as usual. */

static uint16_t i								= 0; 
static uint16_t size 						= REST_MAX_CHUNK_SIZE;
static uint32_t chunk_number 		= 0;
//static uint8_t  more 						= 1;
static uint16_t publicationSize = 0;


PROCESS_THREAD(publication_client, ev, data)
{
  PROCESS_BEGIN();

	printf_color(YEL,"\Publication Client Started \n");
//  sprintf(publication,"[%s][temp 21 grades][%s]",TARGET, capabilityToken);


  while(1) {

  SERVER_NODE(&server_ipaddr);
	memset(publication, 0, 900);
  sprintf(publication,"[%s][{\"temp\": \"21\"}][%s]",TARGET, capabilityToken);

  /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
  coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
  coap_set_header_uri_path(request, "/translator");
	
//	coap_set_header_block1(void *packet, uint32_t num, uint8_t more, uint16_t size);
	i								= 0; 
	size 						= REST_MAX_CHUNK_SIZE;
	chunk_number 		= 0;
	more 						= 1;
	publicationSize = 0;
	publicationSize = strlen(publication);
	
//	printf ("What we are going to publish \n %s \n", publication);	

	
	for(i=0; i<publicationSize; i+=REST_MAX_CHUNK_SIZE){ 

		chunk_number = i/REST_MAX_CHUNK_SIZE;
		more = (i+REST_MAX_CHUNK_SIZE < publicationSize);
		if((i+REST_MAX_CHUNK_SIZE > publicationSize))
			size = publicationSize-i;
					


		coap_set_header_block1(request, chunk_number, more, size);
		coap_set_payload(request, &publication[i],size);
		
		COAP_BLOCKING_REQUEST(&server_ipaddr, UIP_HTONS(20218 + node_id), request, client_chunk_handler);

		if(i%2 == 0)
				printf(". ");

    if(!more)
	  { 
			printf_color(GRN,"OK!\n\n  MESSAGE PUBLISHED IN THE PLATFORM !\n");
			process_post(&boot_pub_manager_server, PROCESS_EVENT_CONTINUE, NULL);	
			//printf("publication client, before Yiedl \n");
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
			//printf_color(YEL,"\Publication Client Continuing \n");
		}

    }
  }

  PROCESS_END();
}

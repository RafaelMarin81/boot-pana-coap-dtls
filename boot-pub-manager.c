#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


#include "boot-pub-manager.h"
#include "rest-engine.h"
#include "publication_client.h"
#include "dtls-client.h"
#include "panatiki.h"



#define STARTED 	 	0
#define COMPLETED 	 	1
#define FAILED 		       -1

#define BOOT_TASK		1
#define NEW_PNR_TASK		2


extern int dtlsFinishedHandshake;
static struct etimer mainTimer;

PROCESS(boot_pub_manager_server, "Bootstrapping-DTLS/Erbium Publication");
AUTOSTART_PROCESSES(&boot_pub_manager_server);

PROCESS_THREAD(boot_pub_manager_server, ev, data)
{

void clearEvent(){
	ev = 0;
	memset(data,0,strlen(data));
}

  PROCESS_BEGIN();

	int state 	= BOOT_TASK;
	int outcome 	= STARTED;

	process_start(&panatiki_process,NULL);

	// 1st we complete the bootstrapping process.	
	while(outcome != COMPLETED){

		YIELD:
		printf("HANDLER: Before Process Yield\n");
		PROCESS_YIELD();
		printf("HANDLER: Event %d , Data: %s \n", ev,data);

		if(outcome != COMPLETED && ev == PROCESS_EVENT_TIMER && etimer_expired(&mainTimer)){
			printf("HANDLER: mainTimer\n");
			process_post(&panatiki_process, PROCESS_EVENT_INIT, 
				     "Requesting to start again the Bootstrapping \0");
			goto YIELD;	
		}

		// Waiting for the bootstrapping to finish
		if(ev == PROCESS_EVENT_CONTINUE){
			if(strstr(data, "failed")){
				printf("Bootstrapping failed\n");
				outcome = FAILED;
				etimer_set(&mainTimer, 5 * CLOCK_SECOND);
				clearEvent();
			}
			else 
			if(strstr(data, "completed")){
				printf("Bootstrapping completed\n");
				outcome = COMPLETED;
				etimer_set(&mainTimer, 10 * CLOCK_SECOND);
				clearEvent();
			}
		}


	}	


	// 2nd we request the CT
	state = NEW_PNR_TASK;
	outcome = STARTED; 

	process_post(&panatiki_process, PROCESS_EVENT_CONTINUE, 
		     "Requesting to get a Capability Token \0");

	while(outcome != COMPLETED){

		CT_YIELD:
		printf("HANDLER: CT: Before Process Yield\n");
		PROCESS_YIELD();
		printf("HANDLER: CT: Event %d , Data: %s \n", ev,data);

		if(outcome != COMPLETED && ev == PROCESS_EVENT_TIMER && etimer_expired(&mainTimer)){
			printf("HANDLER: CT: mainTimer\n");
			process_post(&panatiki_process, PROCESS_EVENT_CONTINUE, 
				     "Requesting to get a Capability Token \0");
			goto CT_YIELD;	
		}

		// Waiting for the bootstrapping to finish
		if(ev == PROCESS_EVENT_CONTINUE){
			if(strstr(data, "failed")){
				printf("Capability Token failed\n");
				outcome = FAILED;
				etimer_set(&mainTimer, 5 * CLOCK_SECOND);
				clearEvent();
			}
			else 
			if(strstr(data, "completed")){
				printf("Capability Token completed\n");
				outcome = COMPLETED;
				etimer_set(&mainTimer, 10 * CLOCK_SECOND);
				clearEvent();
			}
		}
	}	
	
    if(capabilityTokenLen > 0)
	{	
		printf("HANDLER: We Got the Capability Token (%d) \n",capabilityTokenLen);
        printf_hex(capabilityToken,capabilityTokenLen);
	}
	else{
		printf("HANDLER: We DIDN'T Got the Capability Token \n");
	}


//////////////////////////////////

  printf("HANDLER: WE START DTLS\n");
  // Initialize the REST engine.   
  rest_init_engine();

  static struct etimer et;
  etimer_set(&et, 3 * CLOCK_SECOND);
  do {
    PROCESS_YIELD();
    if(etimer_expired(&et) && !dtlsFinishedHandshake) {
      PRINTF("boot-pub-manager waiting\n");
      etimer_restart(&et);
    }
  } while(!dtlsFinishedHandshake);


    process_start(&publication_client,NULL);

    PROCESS_END();
}

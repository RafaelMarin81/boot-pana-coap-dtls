#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "oma_lwm2m.h"

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif

#define DEBUG 1 //DEBUG_PRINT
#include "net/uip-debug.h"

/*#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif*/


#include "dev/serial-line.h"
#include "dev/uart1.h"

#include "boot-pub-manager.h"
#include "rest-engine.h"
#include "er-coap-engine.h"

//#include "publication_client.h"

#include "panatiki.h"



#define STARTED	 		 0
#define COMPLETED 		 1
#define FAILED 		  	-1

#define BOOT_TASK		 1
#define NEW_PNR_TASK		 2


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

	{
		uart1_set_input(serial_line_input_byte);
		serial_line_init();
	}


	//static int state 	= BOOT_TASK;  // No-used
	static int outcome 	= STARTED;

	printf_color(BLU, "\nBootstrapping and Publication in Smart Object\n");

	printf_color(CYN, "-----------------------------------------------\n");
	printf_color(CYN, "PHASE 1: BOOTSTRAPPING: NETWORK AUTHENTICATION \n");
	printf_color(CYN, "-----------------------------------------------\n");
	
/*    printf_color(BLU, "\n WAITING FOR KEY INTERRUPTION \n");	
	while(ev != serial_line_event_message){
		PROCESS_YIELD();
	}*/

	printf_color(CYN, "Starting PANATIKI...\n");

	process_start(&panatiki_process,NULL);		// PANATIKI dows retries to connect until infinite.

	// 1st we complete the bootstrapping process.	
        //etimer_set(&mainTimer, 10 * CLOCK_SECOND);	// 20 seconds to restarting the DTLS connection.
	while(outcome != COMPLETED){

		YIELD:
		printf_color(CYN, "Waiting for the Bootstrapping to complete PANATIKI...\n");
		PROCESS_YIELD();

		if(outcome != COMPLETED && ev == PROCESS_EVENT_TIMER && etimer_expired(&mainTimer)){
			printf_color(CYN, "Retrying Bootstrapping...\n");
			process_post(&panatiki_process, PROCESS_EVENT_INIT,
				     "Requesting to start again the Bootstrapping");
			
			goto YIELD;	
		}

		// Waiting for the bootstrapping to finish
		if(ev == PROCESS_EVENT_FAILURE){
				printf_color(RED,"Failed: ");
				printf_color(RED, data);
				outcome = FAILED;
				etimer_set(&mainTimer, 5 * CLOCK_SECOND);
				clearEvent();
		}
		else if(ev == PROCESS_EVENT_SUCCESS){
				printf_color(GRN,"Success: ");
				printf_color(GRN, data);
				outcome = COMPLETED;
				etimer_set(&mainTimer, 10 * CLOCK_SECOND);
				clearEvent();
		}


	}	

	printf_color(CYN, "Bootstrapping Completed \n");
	// Waiting for keyboard event

/*    printf_color(BLU, "\n WAITING FOR KEY INTERRUPTION \n");	
	while(ev != serial_line_event_message){
		PROCESS_YIELD();
	}*/


  printf_color(YEL, "-----------------------------------------------\n");
  printf_color(YEL, "PHASE 3: PUBLICATION \n");
  printf_color(YEL, "-----------------------------------------------\n");


  printf_color(YEL,"Init the COAP Server. ");

  printf_color(YEL,"Starting DTLS connection... ");
  // Initialize the REST engine.   
  rest_init_engine();

  printf_color(YEL,"Waiting for DTLS to connect... ");

 // static struct etimer et;
  etimer_set(&mainTimer, 20 * CLOCK_SECOND);	// 20 seconds to restarting the DTLS connection.
  do {
    PROCESS_YIELD();
    if(etimer_expired(&mainTimer) && !dtlsFinishedHandshake) {
     printf_color(YEL,"Restarting DTLS connection... ");
     process_post(&coap_engine, PROCESS_EVENT_INIT, "Start again DTLS");

     etimer_restart(&mainTimer);
    }
  } while(!dtlsFinishedHandshake);

   printf_color(GRN,"DTLS connected successfully ");

   // Initialize the OMA LWM2M resources.   
   printf_color(YEL,"Enabling COAP - OMA resources.");
   oma_lwm2m_init();

   // Initialize the Periodic-timers of Periodic-resources.   
   printf_color(YEL,"Initing Periodic Resources.");
   rest_init_periodic_resources();


/*
   printf_color(YEL,"Starting publication ");


 	 process_start(&publication_client,NULL);
     
         etimer_set(&mainTimer, 20 * CLOCK_SECOND);
	 	

	 static int publication_counter = 1;

	 do {
			publication_counter++;
			PROCESS_YIELD();

			if( ev == PROCESS_EVENT_TIMER  && etimer_expired(&mainTimer) && !more){
				process_post(&publication_client, PROCESS_EVENT_CONTINUE, 
						   "Requesting to publish again");
			}
		   etimer_restart(&mainTimer);
	 } while(publication_counter < 20);		// Number of publications.
*/

   /* Define application-specific events here. */
   while(1) {
       PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
       if(ev == sensors_event && data == &button_sensor) {
           PRINTF("*******BUTTON*******\n");
           
           /* Call the event_handler for this application-specific event. */
           res_event.trigger();
           
           /* Also call the separate response example handler. */
           res_separate.resume();
       }
#endif /* PLATFORM_HAS_BUTTON */
   } /* while (1) */

   PROCESS_END();

}

/*
 * Copyright (c) 2015, Jens Eliasson, Lulea University of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is an extension of the Contiki operating system.
 */

/**
 * \file
 *         Resource Directory client
 * \author
 *         Jens Eliasson <jens.eliasson@ltu.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"

#include "er-coap-engine.h"

#ifdef DIGITAL_INPUT_PORT2_PIN
#include "interruptP2.h"
#endif

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#include "rest-engine.h"

#include "rd_client.h"

extern resource_t res_ezequiel_60000_0_Button;
extern int32_t res_ezequiel_60000_0_Button_Counter;
extern int dtlsFinishedHandshake;

PROCESS(rd_client, "OMA LWM2M /rd Client");

/* FIXME: The /rd server address is hard-coded for now. Should be obtained from the /bs server or similar */
// Ezequiel: pongo aaaa::01
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1)

uip_ipaddr_t server_ipaddr;
static struct etimer et;
static int rdstate = 1;

char *service_urls[2] = { "/bs", "/rd"};

char endp[64];

void handler_button(uint8_t pin) {
  res_ezequiel_60000_0_Button.trigger();
  PRINTF("presionado!%lu\n",res_ezequiel_60000_0_Button_Counter);
}

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
  printf("Location-path: %s\n", endp);

  len = coap_get_payload(response, &chunk);
  printf("|%.*s", len, (char *)chunk);
}

PROCESS_THREAD(rd_client, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, 3 * CLOCK_SECOND);
  do {
    PROCESS_YIELD();
    if(etimer_expired(&et) && !dtlsFinishedHandshake) {
      PRINTF("rd_client_waiting\n");
      etimer_restart(&et);
    }
  } while(!dtlsFinishedHandshake);

  // Ezequiel: Preparar las interrupciones del botón:
  #ifdef DIGITAL_INPUT_PORT2_PIN
  interrupt_P2_disable(DIGITAL_INPUT_PORT2_PIN);
  interrupt_P2_set_handler(&handler_button);
  interrupt_P2_enable(&rd_client, INTERRUPT_P2_TRANSITION_HIGH_TO_LO, BUTTON_PORT2_PIN);
  #endif

  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  SERVER_NODE(&server_ipaddr);

  printf("/rd-/sd client\n");

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&et)) {
      if (rdstate == 0) {
        printf("\nInitiate Bootstrap mechanism\n");

        /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, service_urls[0]);
        coap_set_header_uri_query(request, "?ep=mulle-201");

        PRINT6ADDR(&server_ipaddr);
        PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));
        COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request, client_chunk_handler);
        printf("\n--Done--\n");
        rdstate = 1;
      } else if (rdstate == 1) {
        printf("\nRegister at RD\n");

        /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, service_urls[1]);

        const char msg[] = "</60000/0>"; // get this from oma_lwm2m.c registry
        coap_set_payload(request, (uint8_t *) msg, sizeof(msg) - 1);

        coap_set_header_uri_query(request, "?ep=mulle-202"); //FIXME: the name should come from system config

        PRINT6ADDR(&server_ipaddr);
        //PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

        COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request, client_chunk_handler);
        printf("\n--Done--\n");
        rdstate = 2;
      } else {
        //printf("Updating RD using %s\n", endp);
        //coap_set_header_uri_path(request, endp);
        //FIXME: updates is to be added
      }
      etimer_restart(&et);

    }
  }

  PROCESS_END();
}

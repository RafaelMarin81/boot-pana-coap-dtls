diff ../er-coap-dtls/er-coap.c ../contiki-odins-msp430/contiki/apps/er-coap/er-coap.c
46d45
< #include "er-coap-dtls.h"
63c62
< struct uip_udp_conn *udp_conn = NULL;
---
> static struct uip_udp_conn *udp_conn = NULL;
68d66
< 
280,281c278,282
<   // Ezequiel: udp config to DTLS
<   coap_init_dtls(port);
---
>   /* new connection with remote host */
>   udp_conn = udp_new(NULL, 0, NULL);
>   udp_bind(udp_conn, port);
>   PRINTF("Listening on port %u\n", uip_ntohs(udp_conn->lport));
> 
322c323
<                          & (coap_pkt->version) << COAP_HEADER_VERSION_POSITION;
---
>     & (coap_pkt->version) << COAP_HEADER_VERSION_POSITION;
324c325
<                          & (coap_pkt->type) << COAP_HEADER_TYPE_POSITION;
---
>     & (coap_pkt->type) << COAP_HEADER_TYPE_POSITION;
326c327
<                          & (coap_pkt->token_len) << COAP_HEADER_TOKEN_LEN_POSITION;
---
>     & (coap_pkt->token_len) << COAP_HEADER_TOKEN_LEN_POSITION;
418a420,436
> void
> coap_send_message(uip_ipaddr_t *addr, uint16_t port, uint8_t *data,
>                   uint16_t length)
> {
>   /* configure connection to reply to client */
>   uip_ipaddr_copy(&udp_conn->ripaddr, addr);
>   udp_conn->rport = port;
> 
>   uip_udp_packet_send(udp_conn, data, length);
> 
>   PRINTF("-sent UDP datagram (%u)-\n", length);
> 
>   /* restore server socket to allow data from any node */
>   memset(&udp_conn->ripaddr, 0, sizeof(udp_conn->ripaddr));
>   udp_conn->rport = 0;
> }
> /*---------------------------------------------------------------------------*/
432c450
<                       >> COAP_HEADER_VERSION_POSITION;
---
>     >> COAP_HEADER_VERSION_POSITION;
434c452
<                    >> COAP_HEADER_TYPE_POSITION;
---
>     >> COAP_HEADER_TYPE_POSITION;
623c641
<                                 << (coap_pkt->block2_num & 0x07);
---
>         << (coap_pkt->block2_num & 0x07);
634c652
<                                 << (coap_pkt->block1_num & 0x07);
---
>         << (coap_pkt->block1_num & 0x07);
1093c1111
<   if(size < 0) {
---
>   if(size < 16) {
Sólo en ../er-coap-dtls/: er-coap.c~
Sólo en ../er-coap-dtls/: er-coap-dtls.c
Sólo en ../er-coap-dtls/: er-coap-dtls.c~
Sólo en ../er-coap-dtls/: er-coap-dtls.h
Sólo en ../er-coap-dtls/: er-coap-dtls.h~
diff ../er-coap-dtls/er-coap-engine.c ../contiki-odins-msp430/contiki/apps/er-coap/er-coap-engine.c
43,50d42
< #include "er-coap-dtls.h"
< 
< #include "node-id.h"
< 
< extern struct 		uip_udp_conn *udp_conn;
< extern struct 		dtls_context_t *ctx;
< extern int 		dtlsFinishedHandshake;
< extern session_t 	dtls_session;
55c47
< #define PRINTF(...) PRINTF(__VA_ARGS__)
---
> #define PRINTF(...) printf(__VA_ARGS__)
74c66
< int
---
> static int
83c75
<   static coap_packet_t message[1];   /* this way the packet can be treated as pointer as usual */
---
>   static coap_packet_t message[1]; /* this way the packet can be treated as pointer as usual */
88,89d79
<     // TODO: Convertir de DTLS a UDP:
<     decipher_uip_appdata_from_dtls_to_udp();
94a85
> 
175c166
<                       coap_set_payload(response, "BlockOutOfScope", 15);                       /* a const char str[] and sizeof(str) produces larger code size */
---
>                       coap_set_payload(response, "BlockOutOfScope", 15); /* a const char str[] and sizeof(str) produces larger code size */
185c176
<                     }                     /* if(valid offset) */
---
>                     } /* if(valid offset) */
200c191
<                   }                   /* if(resource aware of blockwise) */
---
>                   } /* if(resource aware of blockwise) */
213,216c204,207
<                 }                 /* blockwise transfer handling */
<               }               /* no errors/hooks */
<                               /* successful service callback */
<                               /* serialize response */
---
>                 } /* blockwise transfer handling */
>               } /* no errors/hooks */
>                 /* successful service callback */
>                 /* serialize response */
228,229c219,220
<             coap_error_message = "NoServiceCallbck";             /* no 'a' to fit into 16 bytes */
<           }           /* if(service callback) */
---
>             coap_error_message = "NoServiceCallbck"; /* no 'a' to fit into 16 bytes */
>           } /* if(service callback) */
233c224
<         }         /* if(transaction buffer) */
---
>         } /* if(transaction buffer) */
265,266c256,257
<       }       /* request or response */
<     }     /* parsed correctly */
---
>       } /* request or response */
>     } /* parsed correctly */
332,333d322
< //static struct etimer et;
< 
338d326
< 
340,346d327
<   //BEGIN_AGAIN:
<   PRINTF("BEGIN AGAIN\n");
<   //coap_init_connection(20218 + node_id); //SERVER_LISTEN_PORT);
<   coap_init_connection(20220); //SERVER_LISTEN_PORT);
< 
<   // To force the Router Solicitation.
< 
347a329
>   rest_activate_resource(&res_well_known_core, ".well-known/core");
349,365c331,332
<   while(!dtlsFinishedHandshake) {
<     PRINTF("Client... dtls_connect\n");
<     dtls_connect(ctx, &dtls_session);          
< 
<     PROCESS_YIELD();
<     if(ev == tcpip_event) {
<       dtls_handle_read(ctx);
<     }
<     else if(ev == PROCESS_EVENT_INIT) {
< 	ev = 0;	
< 	PRINTF("PROCESS_EVENT_INIT: Data: %s \n", data);
< 	sendClientHello(ctx, dtls_get_peer(ctx, &dtls_session));	// Retransmit DTLS connection.
<     }
<   }  
< 
<   //rest_activate_resource(&res_well_known_core, ".well-known/core");
<   //coap_register_as_transaction_handler();
---
>   coap_register_as_transaction_handler();
>   coap_init_connection(SERVER_LISTEN_PORT);
368d334
<     
377c343
<   }   /* while (1) */
---
>   } /* while (1) */
Sólo en ../er-coap-dtls/: er-coap-engine.c~
diff ../er-coap-dtls/er-coap-engine.h ../contiki-odins-msp430/contiki/apps/er-coap/er-coap-engine.h
68,69d67
< PROCESS_NAME(coap_engine);
< 
diff ../er-coap-dtls/er-coap.h ../contiki-odins-msp430/contiki/apps/er-coap/er-coap.h
188a189,190
> void coap_send_message(uip_ipaddr_t *addr, uint16_t port, uint8_t *data,
>                        uint16_t length);
diff ../er-coap-dtls/er-coap-observe.c ../contiki-odins-msp430/contiki/apps/er-coap/er-coap-observe.c
236a237
>   static char content[16];
251,258c252,258
<           
<            static char content[16];
<            coap_set_payload(coap_res,
<                              content,
<                              snprintf(content, sizeof(content), "Added %u/%u",
<                                       list_length(observers_list),
<                                       COAP_MAX_OBSERVERS));
<            
---
>           /*
>            * coap_set_payload(coap_res,
>            *                  content,
>            *                  snprintf(content, sizeof(content), "Added %u/%u",
>            *                           list_length(observers_list),
>            *                           COAP_MAX_OBSERVERS));
>            */
Sólo en ../er-coap-dtls/: er-coap-observe.c~
diff ../er-coap-dtls/er-coap-separate.c ../contiki-odins-msp430/contiki/apps/er-coap/er-coap-separate.c
43d42
< #include "er-coap-dtls.h"
Sólo en ../er-coap-dtls/: er-coap-separate.c~
diff ../er-coap-dtls/er-coap-transactions.c ../contiki-odins-msp430/contiki/apps/er-coap/er-coap-transactions.c
43d42
< #include "er-coap-dtls.h"
Sólo en ../er-coap-dtls/: er-coap-transactions.c~
Sólo en ../contiki-odins-msp430/contiki/apps/er-coap/: Makefile.er-coap
Sólo en ../er-coap-dtls/: Makefile.er-coap-dtls

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
 *      OMA LWM2M 7
 * \author
 *      Jens Eliasson <jens.eliasson@ltu.se>
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "utils.h"

#define DEBUG 1
#include "net/uip-debug.h"

#include "dev/leds.h"
#include "sht21_i2c.h"

#include "oma_lwm2m.h"

#ifdef WITH_OMA_LWM2M

void measureMexRF_cast_to(float f32parameter,int16_t output[]) {
	output[0] = (int16_t)f32parameter;
	output[1] = (int16_t)((f32parameter - (float) output[0]) * 100.0);
	output[1] = output[1] < 0 ? -output[1] : output[1];
}

static void res_ezequiel_60000_0_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_ezequiel_60000_0_event(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
int32_t res_ezequiel_60000_0_Button_Counter;
static void res_ezequiel_60000_1_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_ezequiel_60000_1_put(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_ezequiel_60000_2_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_ezequiel_60000_3_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static void res_ezequiel_60000_4_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_ezequiel_60000_4_periodic_handler(void);
static void res_ezequiel_60000_4_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * Macro to define a periodic resource.
 * The corresponding [name]_periodic_handler() function will be called every period.
 * For instance polling a sensor and publishing a changed value to subscribed clients would be done there.
 * The subscriber list will be maintained by the final_handler rest_subscription_handler() (see rest-mapping header file).
 */
#define PERIODIC_RESOURCE(name, attributes, get_handler, post_handler, put_handler, delete_handler, period, periodic_handler) \
      periodic_resource_t periodic_##name; \
  resource_t name = { NULL, NULL, IS_OBSERVABLE | IS_PERIODIC, attributes, get_handler, post_handler, put_handler, delete_handler, { .periodic = &periodic_##name } }; \
  periodic_resource_t periodic_##name = { NULL, &name, period, { { 0 } }, periodic_handler };

/*
#define EVENT_RESOURCE(name, attributes, get_handler, post_handler, put_handler, delete_handler, event_handler) \
      resource_t name = { NULL, NULL, IS_OBSERVABLE, attributes, get_handler, post_handler, put_handler, delete_handler, { .trigger = event_handler } }
*/


//Botón
EVENT_RESOURCE(res_ezequiel_60000_0_Button,
         "title=\"Button_GET_OBSERVE\";obs",
         res_ezequiel_60000_0_get,
         NULL,
         NULL,
         NULL,
         res_ezequiel_60000_0_event);

static void
res_ezequiel_60000_0_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%lu", res_ezequiel_60000_0_Button_Counter));
}

static void
res_ezequiel_60000_0_event(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* Do the update triggered by the event here, e.g., sampling a sensor. */
  ++res_ezequiel_60000_0_Button_Counter;

  /* Usually a condition is defined under with subscribers are notified, e.g., event was above a threshold. */
  PRINTF("Botton %lu", res_ezequiel_60000_0_Button_Counter);
  /* Notify the registered observers which will trigger the res_get_handler to create the response. */
  REST.notify_subscribers(&res_ezequiel_60000_0_Button);
}

//Led
RESOURCE(res_ezequiel_60000_1_Led,
         "title=\"LED_GET_PUT\"",
         res_ezequiel_60000_1_get,
         NULL,
         res_ezequiel_60000_1_put,
         NULL);

int32_t led = 1;

static void
res_ezequiel_60000_1_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%lu", led));
}

static void
res_ezequiel_60000_1_put(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int length;
  uint16_t payload_len = 0;
  uint8_t* payload = NULL;
  char message[80];

  payload_len = REST.get_request_payload(request, (const uint8_t **)&payload);

  if (payload) {
    memcpy(message, payload, payload_len);
    message[payload_len] = '\0';
    PRINTF(" led: %s\n", message);
    if (payload[0] == '0') {
      led = 0;
      leds_off(LEDS_GREEN);
    }
    else {
      led = 1;
      leds_on(LEDS_GREEN);
    }
  }

  //REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_header_etag(response, (uint8_t *)&length, 1);
  //REST.set_response_payload(response, buffer, length);
  REST.set_response_status(response, REST.status.CHANGED);
}

// Temperatura

RESOURCE(res_ezequiel_60000_2_Temperatura,
         "title=\"Temperatura_GET\"",
         res_ezequiel_60000_2_get,
         NULL,
         NULL,
         NULL);

static void
res_ezequiel_60000_2_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  float t,h;
  static int16_t temp[2];
  //static uint8_t n;
  SHT21_readTemperatureAndHumidity(&t, &h);
	measureMexRF_cast_to(t,temp);
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%d.%d", temp[0],temp[1]));
}

// Temperatura

RESOURCE(res_ezequiel_60000_3_Humedad,
	"title=\"Humedad_GET\"",
	res_ezequiel_60000_3_get,
	NULL,
	NULL,
	NULL);

static void
res_ezequiel_60000_3_get(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	float t,h;
	static int16_t humd[2];
	//static uint8_t n;
	SHT21_readTemperatureAndHumidity(&t, &h);
	measureMexRF_cast_to(h,humd);
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%d.%d", humd[0],humd[1]));
}


//Push Periodic OBSERVE
PERIODIC_RESOURCE(res_ezequiel_60000_4_Push,
        "title=\"Periodic Counter\";obs",
        res_ezequiel_60000_4_get_handler,
        NULL,
        res_ezequiel_60000_4_put_handler,
        NULL,
        5 * CLOCK_SECOND,
        res_ezequiel_60000_4_periodic_handler);

static void
res_ezequiel_60000_4_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int length;
  uint16_t payload_len = 0;
  uint8_t* payload = NULL;
  char message[80];
  uint16_t period = 0;

  payload_len = REST.get_request_payload(request, (const uint8_t **)&payload);

  if (payload) {
    memcpy(message, payload, payload_len);
    message[payload_len] = '\0';
    PRINTF("Seconds: %s\n", message);

    period = Utils_asciiDecToInt(message);

    res_ezequiel_60000_4_Push.periodic->period = period * CLOCK_SECOND;

    etimer_stop(&(res_ezequiel_60000_4_Push.periodic->periodic_timer));
    PRINTF("Periodic STOP\n");

    if(res_ezequiel_60000_4_Push.periodic->period != 0) {
        etimer_set(&(res_ezequiel_60000_4_Push.periodic->periodic_timer), res_ezequiel_60000_4_Push.periodic->period);
        etimer_restart(&(res_ezequiel_60000_4_Push.periodic->periodic_timer));
        PRINTF("Periodic SET timer /%s to %ld\n", res_ezequiel_60000_4_Push.url, res_ezequiel_60000_4_Push.periodic->period);
        rest_init_periodic_resources();
    }
    

  }

  //REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_header_etag(response, (uint8_t *)&length, 1);
  //REST.set_response_payload(response, buffer, length);
  REST.set_response_status(response, REST.status.CHANGED);
}


/*
 * Use local resource state that is accessed by res_get_handler() and altered by res_periodic_handler() or PUT or POST.
 */
static int32_t event_counter = 0;

static void
res_ezequiel_60000_4_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    /*
     * For minimal complexity, request query and options should be ignored for GET on observable resources.
     * Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
     * This would be a TODO in the corresponding files in contiki/apps/erbium/!
     */
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_header_max_age(response, res_ezequiel_60000_4_Push.periodic->period / CLOCK_SECOND);
    REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "Counter %lu", event_counter));
    /* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_ezequiel_60000_4_periodic_handler()
{
    /* Do a periodic task here, e.g., sampling a sensor. */
    ++event_counter;
    
    /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
    if(1) {
        /* Notify the registered observers which will trigger the res_get_handler to create the response. */
        REST.notify_subscribers(&res_ezequiel_60000_4_Push);
        PRINTF("Periodic Counter - notify_subscribers %lu", event_counter);
    }
}


#endif

all: boot-pub-manager

CFLAGS+= -DAPP_CEXRF_PLEYADES=1         # Using for definitions of structure for CexRF board.

#============================
# Compilation for Application Layer
#============================

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
#CFLAGS += -DPAA_IP_ADDRESS=0xaaaa,0,0,0,0,0,0,0x1

TARGET=odin5438A

# minimal-net target is currently broken in Contiki
# ifeq ($(TARGET), minimal-net)
CFLAGS += -DHARD_CODED_ADDRESS=\"fdfd::10\"
${info INFO: er-example compiling with large buffers}

CFLAGS += -DUIP_CONF_BUFFER_SIZE=800
CFLAGS += -DCOAP_MAX_HEADER_SIZE=65
CFLAGS += -DQUEUEBUF_CONF_NUM=6        # Number of buffer to save the packet received/transmitted.

CONTIKI_WITH_RPL=0
CFLAGS += -DUIP_CONF_IPV6_RPL=0
#endif

#PROJECTDIRS += 
#PROJECT_SOURCEFILES += publication_client.c

# linker optimizations
SMALL=1

APPS += panatiki
APPS += tinydtls
APPS += er-coap-dtls
APPS += rest-engine


#============================
# Compilation for LWM2M 
#============================

APPS += lwm2m
CONTIKI_SOURCEFILES += interruptP2.c
CONTIKI_SOURCEFILES += serial-line.c ringbuf.c uart1x.c
CONTIKI_SOURCEFILES += sht21_i2c.c  i2c.c

#============================
# Compilation of Network-Stack
#============================

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS += -DWITH_UIP6
CONTIKI_WITH_IPV6 = 1
WITH_DTLS_COAP=1


CFLAGS+= -DRIMEADDR_CONF_SIZE=2     # MAC-address of 2 bytes.  ethebridge considers MAC-Address: 8 bytes and 2 bytes.
CFLAGS += -DBURN_NODEID=0x0002         # MAC-address of mote.

#============================
# Compilation for Radio CC1120
#============================

CONTIKI_SOURCEFILES += cc11xx.c cc1120-msp-arch.c
CONTIKI_SOURCEFILES += serial-line.c ringbuf.c uart1x.c
ARCH_RADIO_SOURCES= cc1120-msp-arch.c
PLATFORMAPPS+=cc1120
CFLAGS += -DCC1120=1
CFLAGS += -DCC11xx_CC1120=1

#============================
# Debug parameters
#============================
#CFLAGS+= -DDEBUG_DATAFLASH_HISTORIC
#CFLAGS+= -DDEBUG_ADC_CLOCK
#CFLAGS+= -DDEBUG_SST25VF032B
#CFLAGS+= -DDEBUG_ERROR
#CFLAGS+= -DDEBUG_ICMP6
#CFLAGS+= -DDEBUG_UIP6
#CFLAGS+= -DDEBUG_ND6
#CFLAGS+= -DDEBUG_TCPIP
#CFLAGS+= -DDEBUG_SICSLOWPAN
#CFLAGS+= -DDEBUG_CSMA
#CFLAGS+= -DDEBUG_NULLRDC
#CFLAGS+= -DDEBUG_FRAMER_802154
#CFLAGS+= -DDEBUG_CC1120
#CFLAGS+= -DDEBUG_CC1120_LEDS
#CFLAGS+= -DDEBUG_INTERRUPT_P1=1


#============================
# Compile parameters
#============================

CONTIKI_HOME = ../contiki-odins-msp430

ifdef CONTIKI_HOME
MIST=$(CONTIKI_HOME)
else
MIST=../..
endif

TARGET = odin5438A
include $(MIST)/Makefile.odin


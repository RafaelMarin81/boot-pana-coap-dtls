#	#	#	#	#	#
#	CONFIGURATION	#
#
#
#	(#) CURRENT_PATH: The path to the folder 
#	(#) Set the variable CONTIKI_ODINS_HOME in ~/.bashrc
#
#	#	#	#	#	#


CURRENT_PATH=/home/rafael/ANASTACIA/contiki-source

#rm -R $CURRENT_PATH/contiki-odins-msp430/contiki/apps/er-coap-dtls
#rm -R $CURRENT_PATH/contiki-odins-msp430/contiki/apps/panatiki
#rm -R $CURRENT_PATH/contiki-odins-msp430/contiki/apps/rest-engine
#rm -R $CURRENT_PATH/contiki-odins-msp430/contiki/apps/tinydtls

#ln -s $CURRENT_PATH/er-coap-dtls $CURRENT_PATH/contiki-odins-msp430/contiki/apps/er-coap-dtls
#ln -s $CURRENT_PATH/panatiki_contikios $CURRENT_PATH/contiki-odins-msp430/contiki/apps/panatiki
#ln -s $CURRENT_PATH/rest-engine $CURRENT_PATH/contiki-odins-msp430/contiki/apps/rest-engine
#ln -s $CURRENT_PATH/tinydtls $CURRENT_PATH/contiki-odins-msp430/contiki/apps/tinydtls

mv $CURRENT_PATH/er-coap-dtls $CURRENT_PATH/contiki-odins-msp430/contiki/apps/er-coap-dtls
mv $CURRENT_PATH/panatiki_contikios $CURRENT_PATH/contiki-odins-msp430/contiki/apps/panatiki
mv $CURRENT_PATH/rest-engine $CURRENT_PATH/contiki-odins-msp430/contiki/apps/rest-engine
mv $CURRENT_PATH/tinydtls $CURRENT_PATH/contiki-odins-msp430/contiki/apps/tinydtls

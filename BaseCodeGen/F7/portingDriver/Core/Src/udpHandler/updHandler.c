/*
 * updHandler.c
 *
 *  Created on: 05-Jul-2020
 *      Author: gmahez
 */

#include <string.h>
#include "udpHandler.h"
#include "lwip/udp.h"
#include "debugConsole.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/api.h"

void udpSocInit( void );


void *status;
static struct netconn *conn;
static struct netbuf *buf;
//struct ip_addr ipaddr;
char buffer[10] = "maaply";


struct udp_pcb * udpPcb;

void udpHandlerTask( void * pvParameters )
{


	(void)pvParameters;

	char retValue = 0;

	//udpSocInit();
	vTaskDelay(1000);

	for(;;)
	{
//		 //IP4_ADDR(&ipaddr, IP_ADDR_BROADCAST);
//		 //IPADDR_BROADCAST
		  conn = netconn_new(NETCONN_UDP);
		  netconn_connect(conn, IP_ADDR_BROADCAST, 50010);
//
//		  /*buffer[0]='h';
//		  buffer[1]='o';
//		  buffer[2]='l';
//		  buffer[3]='a';
//		  buffer[4]='\0';*/
//
		  if( NULL == buf )
		  buf=netbuf_new();
		  else
			  debugText("\nBuffer allocated !");
//
//
		  retValue = netbuf_ref(buf,&buffer,strlen(buffer));
		  if( ERR_OK != retValue )
			  debugText("\nBuff refference failed!\n");
		  else
		  retValue = netconn_send(conn,buf);

		  if( ERR_OK != retValue )
			 debugText("\nSend UDP failed!\n");
//
		  if( NULL != buf)
			  netbuf_delete( buf );

		  netconn_delete(conn);

		vTaskDelay(2000);

	}

}


/*********************************************************************************
 *Name :-
 *Para1:-
 *Return:-N/A
 *Details:-
 **********************************************************************************/
void udpSocInit( void )
{

	udpPcb = udp_new();
		if (udpPcb == NULL)
		{
		        debugText("\nudp_new failed!\n");
		        return;
		 }

		udp_bind(udpPcb, IP_ADDR_ANY, 50010);
}

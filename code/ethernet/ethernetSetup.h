/*******************************************************************
   $File:    ethernetSetup.h
   $Date:    Mon, 07 Aug 2017: 18:28
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(ETHERNETSETUP_H)
#define ETHERNETSETUP_H

#include "lwip/err.h"
#include "lwip/netif.h"

// Macros
/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

/* Define those to better describe your network interface. */
#define IFNAME0 'E'
#define IFNAME1 'D'

// Prototypes
//void ethernetif_input(struct netif *netif);
void netifConfig(struct netif* gnetif);


#endif

/*******************************************************************
   $File:    ethernetSetup.h
   $Date:    Mon, 07 Aug 2017: 18:28
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(ETHERNETSETUP_H)
#define ETHERNETSETUP_H

#include "netif.h"

// Prototypes
void ethernetif_input(struct netif *netif);
void netifConfig(void);


#endif

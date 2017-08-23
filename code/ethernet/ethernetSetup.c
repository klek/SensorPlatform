/*******************************************************************
   $File:    ethernetSetup.c
   $Date:    Mon, 07 Aug 2017: 18:26
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if 1

#include "ethernetSetup.h"

#include "string.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"

#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"

// For debug messages
#include "../debug/logging.h"

// Variables
/*
  @Note: The DMARxDscrTab and DMATxDscrTab must be declared in a non cacheable memory region
         In this example they are declared in the first 256 Byte of SRAM1 memory, so this
         memory region is configured by MPU as a device memory (please refer to MPU_Config() in main.c).

         In this example the ETH buffers are located in the SRAM2 memory, 
         since the data cache is enabled, so cache maintenance operations are mandatory.
 */
#if defined ( __CC_ARM   )
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__((at(0x20020000)));/* Ethernet Rx DMA Descriptors */

ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__((at(0x20020080)));/* Ethernet Tx DMA Descriptors */

uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((at(0x2007C000))); /* Ethernet Receive Buffers */

uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((at(0x2007D7D0))); /* Ethernet Transmit Buffers */

#elif defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4 

#pragma location=0x20020000
__no_init ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB];/* Ethernet Rx DMA Descriptors */

#pragma location=0x20020080
__no_init ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB];/* Ethernet Tx DMA Descriptors */

#pragma location=0x2007C000
__no_init uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; /* Ethernet Receive Buffers */

#pragma location=0x2007D7D0
__no_init uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; /* Ethernet Transmit Buffers */

#elif defined ( __GNUC__ ) /*!< GNU Compiler */

ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__((section(".RxDecripSection")));/* Ethernet Rx DMA Descriptor */

ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__((section(".TxDescripSection")));/* Ethernet Tx DMA Descriptors */

uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((section(".RxarraySection"))); /* Ethernet Receive Buffers */

uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((section(".TxarraySection"))); /* Ethernet Transmit Buffers */

#endif


void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    // Structures
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /*
     * Configure the GPIO-pins used for
     * the ethernet
     * ********************************
     * RMII_REF_CLK -------------> PA1
     * RMII_MDIO ----------------> PA2
     * RMII_MDC -----------------> PC1
     * RMII_MII_CRS_DV ----------> PA7
     * RMII_MII_RXD0 ------------> PC4
     * RMII_MII_RXD1 ------------> PC5
     * RMII_MII_RXER ------------> PG2
     * RMII_MII_TX_EN -----------> PG11
     * RMII_MII_TXD0 ------------> PG13
     * RMII_MII_TXD1 ------------> PB13
     */

    // Configuring PA1, PA2, and PA7
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure PB13
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Configure PC1, PC4 and PC5
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configure PG2, PG11 and PG13
    GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_11 | GPIO_PIN_13;

    // Enable the Ethernet global interrupt
    HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    // Enable the Ethernet clock
    __HAL_RCC_ETH_CLK_ENABLE();

    // Output the HSE clock (should be 25 MHz) on MCO pin (PA8) to clock the physical device
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);
}

// Callback for Rx Transfer complete
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    // Empty for now, might not be needed at all
}

/*
 * Low Level initialization of the Ethernet device
 */
static void low_level_init(struct netif *netif, ETH_HandleTypeDef* EthHandle)
{
    uint8_t macAddress[6] =
        {
            MAC_ADDR0,
            MAC_ADDR1,
            MAC_ADDR2,
            MAC_ADDR3,
            MAC_ADDR4,
            MAC_ADDR5
        };

    EthHandle->Instance = ETH;
    EthHandle->Init.MACAddr = macAddress;
    EthHandle->Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    EthHandle->Init.Speed = ETH_SPEED_100M;
    EthHandle->Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    EthHandle->Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    EthHandle->Init.RxMode = ETH_RXINTERRUPT_MODE;
    EthHandle->Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    EthHandle->Init.PhyAddress = LAN8742A_PHY_ADDRESS;

    // Configure the ethernet peripheral
    if ( HAL_ETH_Init(EthHandle) == HAL_OK )
    {
        // Set the netif link flag
        netif->flags |= NETIF_FLAG_LINK_UP;
    }

    // Initialize the TX descriptors list: Chain Mode
    // NOTE(klek): This must be checked in main.c
    //             Need to setup memory space for this
    HAL_ETH_DMATxDescListInit(EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

    // Initialize the RX descriptors list: Chain mode
    // NOTE(klek): This must be checked in main.c
    //             Need to setup memory space for this
    HAL_ETH_DMARxDescListInit(EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

    // Set netif maximum transfer unit
    netif->mtu = 1500;

    // Set broadcast address and ARP traffic
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    
    // Enable MAC and DMA transmission and reception
    HAL_ETH_Start(EthHandle);
}

/*
 * Low level function to send a packet
 */
static err_t low_level_output(struct netif* netif, struct pbuf* p, ETH_HandleTypeDef* EthHandle)
{
    err_t errval;
    struct pbuf *q;
    uint8_t *buffer = (uint8_t *)(EthHandle->TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;

    DmaTxDesc = EthHandle->TxDesc;
    bufferoffset = 0;
  
    // copy frame from pbufs to driver buffers
    for(q = p; q != NULL; q = q->next)
    {
        // Is this buffer available? If not, goto error
        if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
        {
            errval = ERR_USE;
            goto error;
        }
    
        // Get bytes in current lwIP buffer
        byteslefttocopy = q->len;
        payloadoffset = 0;
        
        // Check if the length of data to copy is bigger than Tx buffer size
        while( (byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE )
        {
            // Copy data to Tx buffer
            memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset) );
            
            // Point to next descriptor
            DmaTxDesc = (ETH_DMADescTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);
            
            // Check if the buffer is available
            if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
            {
                errval = ERR_USE;
                goto error;
            }
            
            buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);
            
            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }
    
        // Copy the remaining bytes
        memcpy( (uint8_t*)((uint8_t*)buffer + bufferoffset), (uint8_t*)((uint8_t*)q->payload + payloadoffset), byteslefttocopy );
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

    // Clean and Invalidate data cache
    SCB_CleanInvalidateDCache ();  
    // Prepare transmit descriptors to give to DMA
    HAL_ETH_TransmitFrame(EthHandle, framelength);
  
    errval = ERR_OK;
    
error:
    
    // When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission
    if ((EthHandle->Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
    {
        // Clear TUS ETHERNET DMA flag
        EthHandle->Instance->DMASR = ETH_DMASR_TUS;
        
        // Resume DMA transmission
        EthHandle->Instance->DMATPDR = 0;
    }
    return errval;

}

/*
 * Low level function to receive a packet
 */
static struct pbuf* low_level_input(struct netif* netif, ETH_HandleTypeDef* EthHandle)
{
    struct pbuf *p = NULL, *q = NULL;
    uint16_t len = 0;
    uint8_t *buffer;
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i=0;
  
    /* get received frame */
    if(HAL_ETH_GetReceivedFrame_IT(EthHandle) != HAL_OK)
        return NULL;
    
    /* Obtain the size of the packet and put it into the "len" variable. */
    len = EthHandle->RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle->RxFrameInfos.buffer;
    
    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }
    
    /* Clean and Invalidate data cache */
    SCB_CleanInvalidateDCache ();
    
    if (p != NULL)
    {
        dmarxdesc = EthHandle->RxFrameInfos.FSRxDesc;
        bufferoffset = 0;
        
        for(q = p; q != NULL; q = q->next)
        {
            byteslefttocopy = q->len;
            payloadoffset = 0;
            
            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
            while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
            {
                /* Copy data to pbuf */
                memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));
                
                /* Point to next descriptor */
                dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
                buffer = (uint8_t *)(dmarxdesc->Buffer1Addr);
                
                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset = 0;
            }
            
            /* Copy remaining data in pbuf */
            memcpy( (uint8_t*)((uint8_t*)q->payload + payloadoffset), (uint8_t*)((uint8_t*)buffer + bufferoffset), byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }
    }
    
    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dmarxdesc = EthHandle->RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i=0; i< EthHandle->RxFrameInfos.SegCount; i++)
    {  
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }
    
    /* Clear Segment_Count */
    EthHandle->RxFrameInfos.SegCount =0;
    
    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((EthHandle->Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        EthHandle->Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        EthHandle->Instance->DMARPDR = 0;
    }
    return p;
    
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t ethernetif_init(struct netif *netif, ETH_HandleTypeDef* EthHandle)
{
//    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

//    ethernetif = mem_malloc(sizeof(struct ethernetif));
//    if (ethernetif == NULL) {
//        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
//        return ERR_MEM;
//    }

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
//    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

//    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

//    ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif, EthHandle);

    return ERR_OK;
}

/*
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(struct netif *netif, ETH_HandleTypeDef* EthHandle)
{
    //struct ethernetif *ethernetif;
    //struct eth_hdr *ethhdr;
    struct pbuf *p;

    //ethernetif = netif->state;

    /* move received packet into a new pbuf */
    p = low_level_input(netif, EthHandle);
    /* if no packet could be read, silently ignore this */
    if (p != NULL) {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK) {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
        }
    }
}

/*
 * @brief  Initializes the lwIP stack
 * @param  None
 * @retval None
 */
void netifConfig(struct netif* gnetif)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    
#ifdef USE_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else
    IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
    IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
    IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  
    /* add the network interface */    
    //netif_add(gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    netif_add(gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernetif_input);
    
    /*  Registers the default network interface. */
    netif_set_default(gnetif);
  
    if (netif_is_link_up(gnetif))
    {
        /* When the netif is fully configured this function must be called.*/
        netif_set_up(gnetif);
    }
    else
    {
        /* When the netif link is down this function must be called */
        netif_set_down(gnetif);
    }
}

#endif

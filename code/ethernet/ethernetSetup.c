/*******************************************************************
   $File:    ethernetSetup.c
   $Date:    Mon, 07 Aug 2017: 18:26
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if 0

#include "ethernetSetup.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"

#include "lwip/opt.h"
#include "lwip/timeouts.h"
//#include "netif/ethernet.h"
//#include "netif/etharp.h"

// For debug messages
#include "../debug/logging.h"


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
    __HAL_RCC_ET_CLK_ENABLE();

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
static void low_level_init(struct netif *netif)
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

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = macAddress;
    EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    EthHandle.Init.Speed = ETH_SPEED_100M;
    EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    EthHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    EthHandle.Init.PhyAddress = LAN8742A_PHY_ADDRESS;

    // Configure the ethernet peripheral
    if ( HAL_ETH_Init(&EthHandle) == HAL_OK )
    {
        // Set the netif link flag
        netif->flags |= NETIF_FLAG_LINK_UP;
    }

    // Initialize the TX descriptors list: Chain Mode
    // NOTE(klek): This must be checked in main.c
    //             Need to setup memory space for this
    HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

    // Initialize the RX descriptors list: Chain mode
    // NOTE(klek): This must be checked in main.c
    //             Need to setup memory space for this
    HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

    // Set netif maximum transfer unit
    netif->mtu = 1500;

    // Set broadcast address and ARP traffic
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    
    // Enable MAC and DMA transmission and reception
    HAL_ETH_Start(&EthHandle);
}

/*
 * Low level function to send a packet
 */
static err_t low_level_output(struct netif* netif, struct pbuf* p)
{
    err_t errval;
    struct pbuf *q;
    uint8_t *buffer = (uint8_t *)(EthHandle.TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;

    DmaTxDesc = EthHandle.TxDesc;
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
    HAL_ETH_TransmitFrame(&EthHandle, framelength);
  
    errval = ERR_OK;
    
error:
    
    // When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission
    if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
    {
        // Clear TUS ETHERNET DMA flag
        EthHandle.Instance->DMASR = ETH_DMASR_TUS;
        
        // Resume DMA transmission
        EthHandle.Instance->DMATPDR = 0;
    }
    return errval;

}

/*
 * Low level function to recieve a packet
 */
static struct pbuf* low_level_input(struct netif* netif)
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
    if(HAL_ETH_GetReceivedFrame_IT(&EthHandle) != HAL_OK)
        return NULL;
    
    /* Obtain the size of the packet and put it into the "len" variable. */
    len = EthHandle.RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;
    
    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }
    
    /* Clean and Invalidate data cache */
    SCB_CleanInvalidateDCache ();
    
    if (p != NULL)
    {
        dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
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
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i=0; i< EthHandle.RxFrameInfos.SegCount; i++)
    {  
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }
    
    /* Clear Segment_Count */
    EthHandle.RxFrameInfos.SegCount =0;
    
    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  
    {
        /* Clear RBUS ETHERNET DMA flag */
        EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        EthHandle.Instance->DMARPDR = 0;
    }
    return p;
    
}

#endif

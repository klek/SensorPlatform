/*******************************************************************
   $File:    ethernet.c
   $Date:    Mon, 07 Aug 2017: 18:26
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "ethernet.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"

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

    // Enable MAC and DMA transmission and reception
    HAL_ETH_Start(&EthHandle);
}

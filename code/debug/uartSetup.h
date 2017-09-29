/*******************************************************************
   $File:    uartSetup.h
   $Date:    Mon, 22 May 2017: 19:00
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(UARTSETUP_H)
#define UARTSETUP_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"
#include "stdio.h"

/* Definition for USARTx clock resources */
#define USARTx                          USART3
#define USARTx_CLK_ENABLE()             __HAL_RCC_USART3_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()

#define USARTx_FORCE_RESET()            __HAL_RCC_USART3_FORCE_RESET()
#define USARTx_RELEASE_RESET()          __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                   GPIO_PIN_8
#define USARTx_TX_GPIO_PORT             GPIOD
#define USARTx_TX_AF                    GPIO_AF7_USART3
#define USARTx_RX_PIN                   GPIO_PIN_9
#define USARTx_RX_GPIO_PORT             GPIOD
#define USARTx_RX_AF                    GPIO_AF7_USART3

#define UART_BAUD_RATE                  115200//9600
// Macro the printf-function

// Macros for packet info
#define PACKET_SIZE						256


void uartSetup(void);
void HAL_UART_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart);
void uartSend(char type, uint8_t* data, uint16_t dataSize);

#endif

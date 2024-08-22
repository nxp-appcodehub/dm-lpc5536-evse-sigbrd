/*
 * Copyright (c) 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMPORTDRIVER_H
#define COMPORTDRIVER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "sigbrd_application.h"

/*!
 * @addtogroup comm_port_driver
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* CommPortDriver specific macros */

/* Host controller port specific */
#define HOST_USART_CONTROL_INIT                  USART0_Init
#define HOST_USART_BASE_PTR_CONTROL              USART0
#define HOST_USART_CONTROL_GETCHAR               USART_ReadByte(USART0)
#define HOST_USART_CONTROL_PUTCHAR(c)            USART_WriteByte(USART0,c)
#define HOST_USART_CONTROL_CLOCK                 BOARD_DEBUG_UART_CLK_FREQ
#define HOST_USART_INTERRUPT_PRIORITY         	 0

/* M_UART controller port specific */
#define M_USART_CONTROL_INIT                     USART1_Init
#define M_USART_BASE_PTR_CONTROL                 USART1
#define M_USART_CONTROL_GETCHAR                  USART_ReadByte(USART1)
#define M_USART_CONTROL_PUTCHAR(c)               USART_WriteByte(USART1,c)
#define M_USART_CONTROL_CLOCK                    BOARD_DEBUG_UART_CLK_FREQ
#define M_USART_INTERRUPT_PRIORITY         		 0

#define EOL                     0x0D
#define zUARTBUFFER             128

#define STX                     0x02
#define ETX                     0x03
#define EOT                     0x04
#define ACK                     0x06
#define END_OF_MSG              '\r'

/*! @brief CommPort_Driver Indexes */
typedef enum
{
  UART_CONTROL_INDEX,
  UART_METER_INDEX,
  NUM_UART_CHANNEL
} UART_INDEX;

enum
{
  UART_TX,
  UART_RX,
};

/*! @brief CommPort_Driver Modes*/
typedef enum
{
  UART_IDLE,
  UART_RXING,
  UART_PROCESSING,
  UART_TXING
} UART_STATUS;


/*******************************************************************************
* Variables
******************************************************************************/
extern volatile uint8  uartRxPortStatus[NUM_UART_CHANNEL];
extern volatile uint8  uartTxPortStatus[NUM_UART_CHANNEL];
extern uint8  uartProtocol[NUM_UART_CHANNEL];
extern uint16 uartRxBufIndex[NUM_UART_CHANNEL];
extern uint16 uartTxBufIndex[NUM_UART_CHANNEL];
extern uint16 uartBytesToTx[NUM_UART_CHANNEL];
extern uint8 *pUartTxBuffer[NUM_UART_CHANNEL];
extern uint8  EVSECommUnlocked;
extern uint8  uartTxBuffer[NUM_UART_CHANNEL][zUARTBUFFER];
extern uint8  uartRxBuffer[NUM_UART_CHANNEL][zUARTBUFFER];
extern uint16 responseBufIndex;

/*! @name CommPort_Driver Configuration */
/*@{*/

/*!
 * @brief Initializes the UART ports for host communication.
 */
void Uart_ModuleInit(void);

/*!
 * @brief Transmits a buffer data.
*
* @param uart_index index if UART instance.
* @param txBuff   	pointer to buffer array.
* @param txLength   Length of data in bytes.
*/
void UART_Transmit(UART_INDEX uart_index, uint8* txBuff, uint16 txLength);

/*!
 * @brief Communicating advertisement/notification to host & meter.
 */
void Comm_Process_Advertisement(void);
/*!
 * @brief Communication process/task.
 */
void Comm_Process(void);
/*@}*/

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif/* _COMM_PORT_DRIVER_H_*/

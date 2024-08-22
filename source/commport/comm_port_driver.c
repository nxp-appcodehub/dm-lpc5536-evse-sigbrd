/*
 * Copyright (c) 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "sigbrd_application.h"
#include "comm_port_driver.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UARTHandlerHost     FLEXCOMM0_IRQHandler
#define UARTHandlerM     	FLEXCOMM1_IRQHandler
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8  uartRxPortStatus[NUM_UART_CHANNEL];
volatile uint8  uartTxPortStatus[NUM_UART_CHANNEL];
uint16 uartRxBufIndex[NUM_UART_CHANNEL];
uint16 uartTxBufIndex[NUM_UART_CHANNEL];
uint16 uartBytesToTx[NUM_UART_CHANNEL];
uint8 *pUartTxBuffer[NUM_UART_CHANNEL];
USART_Type *usart[NUM_UART_CHANNEL] = {HOST_USART_BASE_PTR_CONTROL, M_USART_BASE_PTR_CONTROL};
extern uint16_t g_sleepTimeout;

/*******************************************************************************
 * Prototype
 ******************************************************************************/
/*!
 * @brief Handle the interrupts related to UART index.
 *
 * @param uint_index index of the UART port
 */
void UARTHandler(UART_INDEX uint_index);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * brief Initializes the UART instances which are required for communication
 * with host clients.
 */
void Uart_ModuleInit(void)
{
	usart_config_t usartConfig;

	/* Host UART  */
	/*
	 * uartConfig.baudRate_Bps = 115200;
	 * uartConfig.parityMode = kUART_ParityDisabled;
	 * uartConfig.stopBitCount = kUART_OneStopBit;
	 * uartConfig.txFifoWatermark = 0;
	 * uartConfig.rxFifoWatermark = 1;
	 * uartConfig.enableTx = false;
	 * uartConfig.enableRx = false;
	 */
	USART_GetDefaultConfig(&usartConfig);
	usartConfig.baudRate_Bps = 115200;
	usartConfig.enableTx     = false;
	usartConfig.enableRx     = true;

	USART_Init(HOST_USART_BASE_PTR_CONTROL, &usartConfig, HOST_USART_CONTROL_CLOCK);
	/* Enable RX interrupt. */
	USART_EnableInterrupts(HOST_USART_BASE_PTR_CONTROL, kUSART_RxLevelInterruptEnable);
	uartRxPortStatus[UART_CONTROL_INDEX] = uartTxPortStatus[UART_CONTROL_INDEX] = UART_IDLE;
	uartRxBufIndex[UART_CONTROL_INDEX] = 0;
	uartTxBufIndex[UART_CONTROL_INDEX] = 0;

	NVIC_SetPriority(FLEXCOMM0_IRQn, HOST_USART_INTERRUPT_PRIORITY);
	EnableIRQ(FLEXCOMM0_IRQn);

	/* M_UART  */
	/*
	 * uartConfig.baudRate_Bps = 115200;
	 * uartConfig.parityMode = kUART_ParityDisabled;
	 * uartConfig.stopBitCount = kUART_OneStopBit;
	 * uartConfig.txFifoWatermark = 0;
	 * uartConfig.rxFifoWatermark = 1;
	 * uartConfig.enableTx = false;
	 * uartConfig.enableRx = false;
	 */
	USART_GetDefaultConfig(&usartConfig);
	usartConfig.baudRate_Bps = 115200;
	usartConfig.enableTx     = false;
	usartConfig.enableRx     = true;

	USART_Init(M_USART_BASE_PTR_CONTROL, &usartConfig, M_USART_CONTROL_CLOCK);
	/* Enable RX interrupt. */
	USART_EnableInterrupts(M_USART_BASE_PTR_CONTROL, kUSART_RxLevelInterruptEnable);
	uartRxPortStatus[UART_METER_INDEX] = uartTxPortStatus[UART_METER_INDEX] = UART_IDLE;
	uartRxBufIndex[UART_METER_INDEX] = 0;
	uartTxBufIndex[UART_METER_INDEX] = 0;

	NVIC_SetPriority(FLEXCOMM1_IRQn, M_USART_INTERRUPT_PRIORITY);
	EnableIRQ(FLEXCOMM1_IRQn);
}

/*!
 * @brief Callback function for any UART instances in MCU.
 */
void UARTHandlerHost(void)
{
	UARTHandler(UART_CONTROL_INDEX);
}

void UARTHandlerM(void)
{
	UARTHandler(UART_METER_INDEX);
}

/*!
 * @brief Callback function being invoked from the ISR of the UART instances.
 * - Rx operation: copies data bytes from UART DATA/FIFO to system RAM buffer.
 * Indicates to application tasks about the current state of Rx process.
 * - Tx operation: writes UART DATA register from system RAM buffer.
 * Indicates to application tasks about the current state of Tx process.
 *
 * param uart_index Index of the UART Port.
 */
void UARTHandler(UART_INDEX uart_index)
{
	/* Variable to store the received byte */
	uint8 rxByte;
	volatile uint8 temp;

	/* If new data arrived. */
	if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(usart[uart_index]))
	{
		/* restart the timer */
		g_sleepTimeout = DEEP_SLEEP_TIMEOUT;
		/*restart the advertisement notification*/
		g_SleepNotification = SLEEP_NOTIFICATION_DISABLED;

		rxByte = USART_ReadByte(usart[uart_index]);

		/* Check if its the 1st byte of a receive sequence */
		if (uartRxPortStatus[uart_index] == UART_IDLE)
		{
			uartRxPortStatus[uart_index] = UART_RXING;
		}

		if (uartRxPortStatus[uart_index] == UART_RXING)
		{
			uartRxBuffer[uart_index][uartRxBufIndex[uart_index]++] = rxByte;
			uartRxBufIndex[uart_index] %= zUARTBUFFER;
			if (rxByte == EOL)
			{
				uartRxPortStatus[uart_index] = UART_PROCESSING;
			}
		}

		if (uartRxPortStatus[uart_index] == UART_PROCESSING)
		{
			NVIC_CallIsr(SWISR_VECTOR_0);
		}
	}

	else if ((kUSART_TxFifoEmptyFlag) & USART_GetStatusFlags(usart[uart_index]))
	{
		if (uartBytesToTx[uart_index] == 0)
		{
			uartTxPortStatus[uart_index] = UART_IDLE;
			/* Disable transmit */
			USART_DisableInterrupts(usart[uart_index], kUSART_TxLevelInterruptEnable);
			uartTxBufIndex[uart_index] = 0;
		}
		else
		{
			USART_WriteByte(usart[uart_index],*(pUartTxBuffer[uart_index] + uartTxBufIndex[uart_index]++));
			uartBytesToTx[uart_index]--;
		}
	}

	SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Initiates a character buffer data transmission through UART port.
 * Only the 1st byte is written to UART DATA register while the rest of the buffer 
 * data are sent from the ISR callback.
 *
 * @param uart_index index of the uart channel.
 * @param txBuff     pointer of the transmission buffer.
 * @param txLength   length of the packet to be transmitted.
 */
void UART_Transmit(UART_INDEX uart_index, uint8* txBuff, uint16 txLength)
{
	pUartTxBuffer[uart_index] = txBuff;
	uartBytesToTx[uart_index] = txLength;
	uartRxPortStatus[uart_index] = UART_IDLE;
	uartTxPortStatus[uart_index] = UART_TXING;
	/* Enable transmit */
	USART_EnableInterrupts(usart[uart_index], kUSART_TxLevelInterruptEnable);
	uartTxBufIndex[uart_index] = 0;
}

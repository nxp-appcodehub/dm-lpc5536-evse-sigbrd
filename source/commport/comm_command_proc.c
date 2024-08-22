/*
 * Copyright (c) 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "stdio.h"
#include "sigbrd_application.h"
#include "comm_port_driver.h"
#include "math.h"
#include "pin_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8  uartRxBuffer[NUM_UART_CHANNEL][zUARTBUFFER];
uint8  uartTxBuffer[NUM_UART_CHANNEL][zUARTBUFFER];
/*Variable to manage the count of Response packets*/
uint16 responseBufIndex;
/*Variable to manage the count of Advertisement packets*/
uint16 advertiseBufIndex;
/*Variable to manage the count of Transmitting packets*/
uint16 transmitBufIndex;
bool respondingMetCmd = false;
char *g_firmwareVersion;
/*Managing the Sleep Notification sent to host*/
SLEEP_NOTIFICATION_STATUS g_SleepNotification = SLEEP_NOTIFICATION_DISABLED;
extern uint16_t g_sleepTimeout;
extern uint16_t g_uartRxTimeout;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief It advertises the Proximity Pilot status to host whenever the value changes.
 */
void Advertise_PPStatus(void);

/*!
 * @brief It advertises the Ground Fault Pilot status to host whenever the value changes.
 */
void Advertise_GFCIStatus(void);

/*!
 * @brief It sends the notification to host before entering into Deep Sleep Mode.
 */
void Advertise_SleepNotificationToHost(void);

/*!
 * @brief It sends the notification to meter whenever the charging status changes.
 *        I=0, P=0 if no charging
 */
void Send_ChargingStatusToMeter(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief This function/task takes care of the advertisement/notification to host & meter
 * - For Deep Sleep, GFCI, Proximity Pilot Detection
 * - It also sends the charging status to Meter board
 */
void Comm_Process_Advertisement(void)
{

	/*Check for asynchronous response */
	if((uartRxPortStatus[UART_CONTROL_INDEX] == UART_IDLE) && (uartTxPortStatus[UART_CONTROL_INDEX] == UART_IDLE))
	{
        /*If the counter set by user to put the device into sleep mode reaches '0'*/
		if(g_sleepTimeout == 0)
		{
			if(g_SleepNotification == SLEEP_NOTIFICATION_DISABLED)
			{
				/*Advertise to host before entering into deep sleep mode*/
				Advertise_SleepNotificationToHost();
				g_SleepNotification = SLEEP_NOTIFICATION_ENABLED;
			}

			else if(g_SleepNotification == SLEEP_NOTIFICATION_ENABLED)
			{
				/*Sleep Notification sent*/
				g_SleepNotification = SLEEP_NOTIFICATION_SENT;
			}
		}

		else if(g_advertiseGFCIState == true)
		{
			g_advertiseGFCIState = false;
			Advertise_GFCIStatus();
		}

		else if(g_advertisePPState == true)
		{
			g_advertisePPState = false;
			Advertise_PPStatus();
		}

		else if(g_sendUpdateToMeter == true)
		{
			g_sendUpdateToMeter = false;
			Send_ChargingStatusToMeter();
		}

	}

}

/*!
 * @brief This function/task takes care of all communication specific sub-activities.
 * - Send the response to communication client for GFCI, PP, CP, ADC, Version and UNACK to unknown commands
 * - Checks whether the meter is powered up enough with supply voltage to sustain communication
 * - Executes any proprietary command for the meter and responds to the communication client.
 */
void Comm_Process(void)
{
	uint8  i;
	int16 ret_val;

	for(i=0; i<NUM_UART_CHANNEL; i++)
	{
		if (uartRxPortStatus[i] == UART_PROCESSING)
		{
			if(g_uartRxTimeout == 0)
			{
				respondingMetCmd = false;
				g_uartRxTimeout = DEMO_UART_RX_PORT_TIMEOUT;
			}
			break;
		}
	}

	if(i == NUM_UART_CHANNEL)
	{
		return;
	}

	if(!respondingMetCmd)
	{
		responseBufIndex = 0;

		/* Command parsing start */
		/* commands '0' to '9' for M_UART pass-through */
		/* commands 'a' to 'z' for EVSE sig brd. */
		switch(uartRxBuffer[i][0])
		{
		/* commands 'a' to 'j' for EVSE sig brd. get commands */
		case 'b':
			/* get command */
			uartTxBuffer[i][responseBufIndex++] = '0' + g_PPState;
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'c':
			/* get command */
			uartTxBuffer[i][responseBufIndex++] = '0' + g_CPState;
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'd':
			/* get command */
			uartTxBuffer[i][responseBufIndex++] = '0' + gfciValue;
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'e':
			/* get command */
			/* get CP ADC value */
			sprintf(&uartTxBuffer[i][responseBufIndex],  "%05d", g_LpadcChnCPResultConfigStruct.convValue);
			responseBufIndex += 5;
			uartTxBuffer[UART_CONTROL_INDEX][responseBufIndex++] = '[';
			uartTxBuffer[UART_CONTROL_INDEX][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[UART_CONTROL_INDEX][responseBufIndex++] = ']';
			uartTxBuffer[UART_CONTROL_INDEX][responseBufIndex++] = END_OF_MSG;
			break;

		case 'f':
			/* get command */
			/* get PP ADC value */
			responseBufIndex += sprintf(&uartTxBuffer[i][responseBufIndex],  "%05d[f]\r", g_LpadcChnPPResultConfigStruct.convValue);
			break;

			/* commands 'i' to 'z' for EVSE sigbrd set commands */
		case 'i':
			/* set command */
			/* duty cycle in uint16_t 4 chars */
			g_dutyCycle = (uartRxBuffer[i][1] - '0') * 10000;
			g_dutyCycle = g_dutyCycle + ((uartRxBuffer[i][2] - '0') * 1000);
			g_dutyCycle = g_dutyCycle + ((uartRxBuffer[i][3] - '0') * 100);
			g_dutyCycle = g_dutyCycle + ((uartRxBuffer[i][4] - '0') * 10);
			g_dutyCycle = g_dutyCycle + ((uartRxBuffer[i][5] - '0') * 1);
			CP_SetDutyCycle(g_dutyCycle);
			uartTxBuffer[i][responseBufIndex++] = '0';	// Null
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'j':
			/* set command */
			/* Close relay */
			Relay_Close();
			uartTxBuffer[i][responseBufIndex++] = '0';	// Null
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'k':
			/* set command */
			/* Open relay */
			Relay_Open();
			uartTxBuffer[i][responseBufIndex++] = '0';	// Null
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;

		case 'v':
		{
			g_firmwareVersion = SIGBOARD_VERSION;
            while(*g_firmwareVersion != '\0')
            {
              uartTxBuffer[i][responseBufIndex++]= *g_firmwareVersion;
              g_firmwareVersion++;

            }
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
            break;
		}

		case 'w':
		{
			uartTxBuffer[i][responseBufIndex++]= g_sigboardHWVer;
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = uartRxBuffer[i][0];
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;
			break;
		}

		case '0':	/* I, U, P commands */
		case '1':	/* I */
		case '2':	/* U */
		case '3':	/* P */
			/* commands '0' to '9' for meter commands */
			if(i == UART_CONTROL_INDEX)
			{
				/* Pass-through control port RDX data to meter port TXD */
				memcpy(uartTxBuffer[UART_METER_INDEX],
						uartRxBuffer[UART_CONTROL_INDEX],
						uartRxBufIndex[UART_CONTROL_INDEX]);

				respondingMetCmd = true;
			}
			break;

		default:
			/*Sending NACK for unrecognized commands*/
			uartTxBuffer[i][responseBufIndex++] = '[';
			uartTxBuffer[i][responseBufIndex++] = 'n';
			uartTxBuffer[i][responseBufIndex++] = ']';
			uartTxBuffer[i][responseBufIndex++] = END_OF_MSG;

			break;
		}

		if(respondingMetCmd)
		{
			UART_Transmit(UART_METER_INDEX, uartTxBuffer[UART_METER_INDEX], uartRxBufIndex[UART_CONTROL_INDEX]);
		}
		else
		{
			UART_Transmit(UART_CONTROL_INDEX, uartTxBuffer[UART_CONTROL_INDEX], responseBufIndex);
		}

		uartRxPortStatus[UART_CONTROL_INDEX] = UART_IDLE;
		g_uartRxTimeout = DEMO_UART_RX_PORT_TIMEOUT;
		uartRxBufIndex[i] = 0;
	}
	else
	{
		if(i == UART_METER_INDEX)
		{
			/* response received from meter UART RXD port */
			/* Pass-through control port TDX data to meter port RXD */
			memcpy(uartTxBuffer[UART_CONTROL_INDEX],
					uartRxBuffer[UART_METER_INDEX],
					uartRxBufIndex[UART_METER_INDEX]);

			UART_Transmit(UART_CONTROL_INDEX, uartTxBuffer[UART_CONTROL_INDEX], uartRxBufIndex[UART_METER_INDEX]);
			respondingMetCmd = false;
			uartRxPortStatus[UART_METER_INDEX] = UART_IDLE;
			uartRxBufIndex[UART_METER_INDEX] = 0;
			g_uartRxTimeout=DEMO_UART_RX_PORT_TIMEOUT;
		}
	}

}


/*!
 * @brief It sends the notification to meter whenever the charging status changes.
 *        I=0, P=0 if no charging else it will continue to show the real time value
 */
void Send_ChargingStatusToMeter(void)
{
	transmitBufIndex=0;
	if (g_chargingCurrentState == true)
	{
		uartTxBuffer[UART_METER_INDEX][transmitBufIndex++] = '5';
	}

	else
	{
		uartTxBuffer[UART_METER_INDEX][transmitBufIndex++] = '6';
	}

	uartTxBuffer[UART_METER_INDEX][transmitBufIndex++] = END_OF_MSG;
	UART_Transmit(UART_METER_INDEX, uartTxBuffer[UART_METER_INDEX], transmitBufIndex);
}

/*!
 * @brief It advertises the Proximity Pilot status to host whenever the value changes.
 */
void Advertise_PPStatus(void)
{
	advertiseBufIndex=0;
	/* get PP ADC value */
	sprintf(&uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex],  "%05d[f]\r", g_LpadcChnPPResultConfigStruct.convValue);
	UART_Transmit(UART_CONTROL_INDEX, uartTxBuffer[UART_CONTROL_INDEX], advertiseBufIndex);
}

/*!
 * @brief It advertises the Ground Fault Pilot status to host whenever the value changes.
 */
void Advertise_GFCIStatus(void)
{
	advertiseBufIndex=0;
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = '0' + gfciValue;
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = '[';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = 'd';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = ']';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = END_OF_MSG;
	UART_Transmit(UART_CONTROL_INDEX, uartTxBuffer[UART_CONTROL_INDEX], advertiseBufIndex);
}

/*!
 * @brief It sends the notification to host before entering into Deep Sleep Mode.
 */
void Advertise_SleepNotificationToHost(void)
{
	advertiseBufIndex=0;
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = '[';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = 'l';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = ']';
	uartTxBuffer[UART_CONTROL_INDEX][advertiseBufIndex++] = END_OF_MSG;
	UART_Transmit(UART_CONTROL_INDEX, uartTxBuffer[UART_CONTROL_INDEX], advertiseBufIndex);
}


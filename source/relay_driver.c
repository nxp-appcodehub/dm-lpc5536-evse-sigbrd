/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "sigbrd_application.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t emStopValue = 0;
bool relayClosedState = false;
bool oldRelayClosedState = false;
uint32_t oldEMStopValue;
RELAY_CALLBACK g_relayAppCallback;

const uint32_t gpio_ip_config = (/* Pin is configured as PIO1_30 */
        		IOCON_PIO_FUNC0 |
				/* Selects pull-up function */
				IOCON_PIO_MODE_PULLUP |
				/* Standard mode, output slew rate control is enabled */
				IOCON_PIO_SLEW_STANDARD |
				/* Input function is not inverted */
				IOCON_PIO_INV_DI |
				/* Enables digital function */
				IOCON_PIO_DIGITAL_EN |
				/* Open drain is disabled */
				IOCON_PIO_OPENDRAIN_DI |
				/* Analog switch 0 is open (disabled) */
				IOCON_PIO_ASW0_DI);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initializes the Relay driver.
 *
 * This function initialize the C variable for relay driver handling.
 *
 */
void Relay_Init(void)
{
	emStopValue = 0;
	relayClosedState = false;
	oldRelayClosedState = false;
}

/*!
 * @brief Register callback function for Relay driver.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by RELAY_INFOTYPE.
 *
 * @param appCallback application Callback.
 */
void Relay_RegisterAppCallback(RELAY_CALLBACK appCallback)
{
	g_relayAppCallback = appCallback;
}

/*!
 * @brief Open Relay.
 *
 * This function opens relay connection.
 *
 */
void Relay_Open(void)
{
	GPIO_PortSet(GPIO, BOARD_RELAY_OFF_PORT, 1u << BOARD_RELAY_OFF_PIN);	// assert RELAY_OFF
	SDK_DelayAtLeastUs((3000), BOARD_BOOTCLOCKPLL150M_CORE_CLOCK);
	GPIO_PortClear(GPIO, BOARD_RELAY_OFF_PORT, 1u << BOARD_RELAY_OFF_PIN);	// withdraw RELAY_OFF
	SDK_DelayAtLeastUs((3000), BOARD_BOOTCLOCKPLL150M_CORE_CLOCK);
	relayClosedState = false;
}

/*!
 * @brief Close Relay.
 *
 * This function closes relay connection.
 *
 */
void Relay_Close(void)
{
	if(g_sigboardHWVer == EVSE_SIGBRD_1X)
	{
	  GPIO_PortClear(GPIO, BOARD_RELAY_ON_PORT, 1u << BOARD_RELAY_ON_PIN_1X); // assert RELAY_ON
	  SDK_DelayAtLeastUs((3000), BOARD_BOOTCLOCKPLL150M_CORE_CLOCK);
	  GPIO_PortSet(GPIO, BOARD_RELAY_ON_PORT, 1u << BOARD_RELAY_ON_PIN_1X); // withdraw RELAY_ON
	}

	else if(g_sigboardHWVer == EVSE_SIGBRD_2X)
	{
	  GPIO_PortClear(GPIO, BOARD_RELAY_ON_PORT, 1u << BOARD_RELAY_ON_PIN_2X); // assert RELAY_ON
	  SDK_DelayAtLeastUs((3000), BOARD_BOOTCLOCKPLL150M_CORE_CLOCK);
	  GPIO_PortSet(GPIO, BOARD_RELAY_ON_PORT, 1u << BOARD_RELAY_ON_PIN_2X); // withdraw RELAY_ON
	}

	relayClosedState = true;
}

/*!
 * @brief Process Relay.
 *
 * This function process relay states.
 *
 */
void Relay_Process(void)
{
	emStopValue = GPIO_PinRead(GPIO, 0U, 14U);

	if(emStopValue != oldEMStopValue)
	{
		if(g_relayAppCallback != NULL)
		{
			g_relayAppCallback(RELAY_INFOTYPE_EM_STOP, emStopValue);
		}
		oldEMStopValue = emStopValue;
	}

	if(relayClosedState != oldRelayClosedState)
	{
		if(g_relayAppCallback != NULL)
		{
			g_relayAppCallback(RELAY_INFOTYPE_STATE_CHANGED, relayClosedState);
		}
		oldRelayClosedState = relayClosedState;
	}
}

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
#include "comm_port_driver.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GFCI_INT_PIN_INT0_SRC     kINPUTMUX_GpioPort1Pin30ToPintsel

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_GFCIOccurred = false;
uint32_t gfciValue;
GFCI_CALLBACK g_gfciAppCallback;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initializes the GFCI.
 *
 * This function initializes the GFCI pin and interrupt.
 *
 */
void GFCI_Init(void)
{
	/* Select ADC0 trigger input from PWM1_SM0_MUX_TRIG0 */
	INPUTMUX->ADC0_TRIG[0] = INPUTMUX_ADC0_TRIGN_ADC0_TRIG_TRIGIN(0x20);

	/* Connect GFCI_INT to PINT */
	INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, GFCI_INT_PIN_INT0_SRC);

	INPUTMUX_Deinit(INPUTMUX);
	/* Setup Pin Interrupt 0 for rising edge */
	PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableBothEdges, pint_intr_callback);
	/* Enable callback for PINT0 by Index */
	PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
}

/*!
 * @brief Register callback function for GFCI.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by GFCI_INFOTYPE.
 *
 */
void GFCI_RegisterAppCallback(GFCI_CALLBACK appCallback)
{
	g_gfciAppCallback = appCallback;
}


/*!
 * @brief Process GFCI.
 *
 * This function Process GFCI state.
 *
 */
void GFCI_Process(void)
{
	if (g_GFCIOccurred)
	{
		if(g_gfciAppCallback != NULL)
		{
			g_gfciAppCallback(GFCI_INFOTYPE_STATE_CHANGED, gfciValue);
		}
		g_GFCIOccurred = false;
	}
}


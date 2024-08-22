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
#define PP_DETECTED_LEVEL_MIN	2000	/* 2V at PP_DET */
#define PP_DETECTED_150R_MIN	16000
#define PP_DETECTED_150R_MAX	22000
#define PP_DETECTED_480R_MIN	30000
#define PP_DETECTED_480R_MAX	38000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
kPPState g_PPState;
kPPState g_oldPPState;
volatile kPPState g_lastPPState;
volatile uint16_t ppVal;
volatile uint32_t ppValRead;
PP_CALLBACK g_ppAppCallback;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initializes the Proximity Pilot.
 *
 * This function initializes the variables for Proximity pilot handling.
 *
 */
void PP_Init(void)
{
	g_PPState = kPPNotDetected;
	g_oldPPState = kPPNotDetected;
}

/*!
 * @brief Register callback function for Proximity Pilot.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by PP_INFOTYPE.
 *
 */
void PP_RegisterAppCallback(PP_CALLBACK appCallback)
{
	g_ppAppCallback = appCallback;
}

/*!
 * @brief Process Proximity Pilot.
 *
 * This function analyze the state of proximity pilot signal.
 *
 */
void PP_Process(void)
{
	ppValRead = GPIO_PinRead(GPIO, BOARD_PP_WAKEUP_PORT, BOARD_PP_WAKEUP_PIN);

	lpadc_conv_result_t mLpadcResultConfigStruct;
	if (!g_LpadcChnPPConversionCompletedFlag)
	{
		return;
	}

	ppVal = g_LpadcChnPPResultConfigStruct.convValue;

	g_LpadcChnPPConversionCompletedFlag = false;

	g_lastPPState = g_PPState;
	if (ppVal < PP_DETECTED_LEVEL_MIN)
	{
		g_PPState = kPPNotDetected;
	}
	else if ((ppVal >= PP_DETECTED_150R_MIN) && (ppVal < PP_DETECTED_150R_MAX))
	{
		g_PPState = kPP150RDetected;
	}
	else if ((ppVal >= PP_DETECTED_480R_MIN) && (ppVal < PP_DETECTED_480R_MAX))
	{
		g_PPState = kPP480RDetected;
	}

	if(g_PPState != g_oldPPState)
	{
		if(g_ppAppCallback != NULL)
		{
			g_ppAppCallback(PP_INFOTYPE_STATE_CHANGED, g_oldPPState, g_PPState);
		}
		g_oldPPState = g_PPState;
	}
}

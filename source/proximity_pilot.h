/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PROXMIMITY_PILOT_H__
#define PROXMIMITY_PILOT_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "types.h"
#include "board.h"
#include "fsl_lpadc.h"

#include "fsl_clock.h"
#include "fsl_inputmux.h"
#include "fsl_anactrl.h"
#include "fsl_ctimer.h"

#include "proximity_pilot.h"

/*!
 * @addtogroup proximity_pilot
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
	kPPNotDetected,
	kPP150RDetected,
	kPP480RDetected,
	kPPDetectedMaxVal,
}kPPState;

/* Info types for application callback function */
typedef enum
{
    PP_INFOTYPE_STATE_CHANGED,      /*!< Pilot Control line voltage state changed. */
} PP_INFOTYPE;

/* Function Pointer to get the PP info type */
typedef void (*PP_CALLBACK)(PP_INFOTYPE kInfoType, uint32_t param1, uint32_t param2);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern volatile bool g_LpadcChnPPConversionCompletedFlag;
extern lpadc_conv_result_t g_LpadcChnPPResultConfigStruct;
extern kPPState g_PPState;
extern volatile kPPState g_lastPPState;
extern lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
extern volatile uint16_t ppVal;
extern volatile uint32_t ppState;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the Proximity Pilot.
 *
 * This function initializes the variables for Proximity pilot handling.
 *
 */
void PP_Init(void);

/*!
 * @brief Register callback function for Proximity Pilot.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by PP_INFOTYPE.
 *
 */
void PP_RegisterAppCallback(PP_CALLBACK appCallback);

/*!
 * @brief Process Proximity Pilot.
 *
 * This function analyze the state of proximity pilot signal.
 *
 */
void PP_Process(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */

#endif /* PROXMIMITY_PILOT_H__ */

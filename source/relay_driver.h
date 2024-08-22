/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RELAY_DRIVER_H__
#define RELAY_DRIVER_H__

#if defined(__cplusplus)
extern "C" {
#endif
#include "types.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_inputmux.h"
#include "fsl_anactrl.h"
#include "fsl_pint.h"

/*!
 * @addtogroup relay_driver
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* info types for application callback function */
typedef enum
{
    RELAY_INFOTYPE_STATE_CHANGED,      	/*!< Relay opened/closed. */
    RELAY_INFOTYPE_EM_STOP,      		/*!< Relay Emergency stop. */
} RELAY_INFOTYPE;

typedef void (*RELAY_CALLBACK)(RELAY_INFOTYPE kInfoType, uint32_t param);


/*******************************************************************************
 * Variables
 ******************************************************************************/

extern volatile uint8_t onRelay;
extern volatile uint8_t offRelay;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the Relay driver.
 *
 * This function initialize the C variable for relay driver handling.
 *
 */
void Relay_Init(void);

/*!
 * @brief Register callback function for Relay driver.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by RELAY_INFOTYPE.
 *
 * @param appCallback application Callback.
 */
void Relay_RegisterAppCallback(RELAY_CALLBACK appCallback);

/*!
 * @brief Open Relay.
 *
 * This function opens relay connection.
 *
 */
void Relay_Open(void);

/*!
 * @brief Close Relay.
 *
 * This function closes relay connection.
 *
 */
void Relay_Close(void);

/*!
 * @brief Process Relay.
 *
 * This function process relay states.
 *
 */
void Relay_Process(void);
#if defined(__cplusplus)
}
#endif


/*!
 * @}
 */

#endif /* RELAY_DRIVER_H__ */

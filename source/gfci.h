/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GFCI_H__
#define GFCI_H__

#if defined(__cplusplus)
extern "C" {
#endif
#include "types.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"

/*!
 * @addtogroup gfci
 * @{
 */

/*! @file */
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* info types for application callback function */
typedef enum
{
    GFCI_INFOTYPE_STATE_CHANGED,      /*!< GFCI state changed. */
} GFCI_INFOTYPE;

typedef void (*GFCI_CALLBACK)(GFCI_INFOTYPE kInfoType, uint32_t param);
extern volatile bool g_GFCIOccurred;
extern uint32_t gfciValue;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the GFCI.
 *
 * This function initializes the GFCI pin and interrupt.
 *
 */
void GFCI_Init(void);

/*!
 * @brief Register callback function for GFCI.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by GFCI_INFOTYPE.
 *
 */
void GFCI_RegisterAppCallback(GFCI_CALLBACK appCallback);

/*!
 * @brief Process GFCI.
 *
 * This function Process GFCI state.
 *
 */
void GFCI_Process(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* GFCI_H__ */

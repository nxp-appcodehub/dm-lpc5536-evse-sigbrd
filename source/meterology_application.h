/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef METEROLOGY_APPLICATION_H__
#define METEROLOGY_APPLICATION_H__

#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * @addtogroup meterology_application
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef ENABLE_METROLOGY
/*!
 * @brief Meterology Initialization
 */
void Metering_Init(void);

/*!
 * @brief Meterology Processing
 */
void Metering_Process(void);
#endif


/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */

#endif /* METEROLOGY_APPLICATION_H__ */

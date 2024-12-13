/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EXTERNAL_AFE_H_
#define EXTERNAL_AFE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "metrology_application.h"
/*!
 * @addtogroup external_afe
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if AFE_MODE == 1

#define AFE_SAMPLES			3200U	/* External AFE is pre-configured to generate
									   3200 samples per second at a constant rate. */
#define DEFAULT_LINE_FREQ	50U		/* Typically AC line frequency is 50 Hertz */
/* There will be AFE_SAMPLES/DEFAULT_LINE_FREQ = 64 samples captured by external AFE for each
   channel. FFT algorithm will need these 64 samples buffered for a cycle to
   start FFT metrology without interpolation. */
#define AFE_INP_SAMPLES (AFE_SAMPLES/DEFAULT_LINE_FREQ)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Initialize SPI and DMA channels for external AFE interface.
 *
 * This function can be used to initialize the SPI master and DMA channels
 * to drive and get data from external AFE SPI slave.
 *
 */
void AFE_Init(void);
void AFE_DeInit(void);
/*!
 * @brief This function will provide status of SPI detection.
 *
 */
bool AFE_SPIDetected(void);
void AFE_SetSPIDetection(bool detection);
#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */

#endif /* EXTERNAL_AFE_H_ */

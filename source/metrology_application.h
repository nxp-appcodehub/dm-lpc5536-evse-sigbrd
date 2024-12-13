/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef METEROLOGY_APPLICATION_H__
#define METEROLOGY_APPLICATION_H__

#include "types.h"
#include "meterlibfft.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * @addtogroup metrology_application
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_DISTRIBUTION_FREQUENCY         50U    /* The frequency is 50Hz */

/*Metering*/
#define FFT_SAMPLES  SAMPLES64    /* number of required FFT samples (power-of-two) */
#define AUX_SAMPLES  (FFT_SAMPLES / 2)       /*!< Number of samples for one FFT calculation */
/* AFE_MODE=1 - asynchronous mode, sampling isn't synchronized with the mains, AFE is running in
                the S/W-triggered continuous conversion mode
   AFE_MODE=0 - synchronous mode, sampling is synchronized with the mains, AFE is running in the
                H/W-triggered single conversion mode */
#define AFE_MODE     1

#define TMRCLK 2998272 /* timer clock base (MHz) */

#define NUM_MET_BUFFERS 2

#define DEMO_PHASE							3U

/*******************************************************************************
 * Conditioned settings                                                        *
 ******************************************************************************/

typedef enum
{
	kMetAFEMode,
	kMetADCMode,
	kMetUARTMode,
	kMetModeSize = kMetUARTMode
}met_mode_t;

/*******************************************************************************
 * Variables                                                                   *
 ******************************************************************************/
extern double urms1[kMetModeSize],irms1[kMetModeSize],p1[kMetModeSize];/* non-billing (instantaneous) metering values Ph1*/
extern double urms2[kMetModeSize],irms2[kMetModeSize],p2[kMetModeSize];/* non-billing (instantaneous) metering values Ph2*/
extern double urms3[kMetModeSize],irms3[kMetModeSize],p3[kMetModeSize];/* non-billing (instantaneous) metering values Ph3*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Meterology Initialization
 */
void Metering_Init(met_mode_t met_mode);

/*!
 * @brief Meterology Processing
 */
void Metering_Process(void* metering_mode);

/**
 * @brief 
 * 
 */
void Metering_SampleFinish(met_mode_t met_mode);

/**
 * @brief Reset the current buffer to the beginning all data added will pe lost.
 * 
 */
void Metering_SampleReset(met_mode_t met_mode);

/**
 * @brief Add sample in the buffer. 
 * 
 * @param u1 Phase 1 Voltage
 * @param u2 Phase 2 Voltage
 * @param u3 Phase 3 Voltage
 * @param i1 Phase 1 Current
 * @param i2 Phase 2 Current
 * @param i3 Phase 3 Current
 * @return uint32_t Return the number of samples already added
 */
uint32_t Metering_SampleAdd(met_mode_t met_mode, frac24 u1, frac24 u2, frac24 u3, frac24 i1, frac24 i2, frac24 i3);

/*!
 * @brief Get Metrology Mode
 */
met_mode_t Metering_GetMetrologyMode(void);

/*!
 * @brief Configure Metrology Mode This function will handle the HW deinit and init.
 */
void Metering_ConfigureMetrologyMode(met_mode_t metrology_mode);

/*!
 * @brief Set Metrology Mode This function will update the metrology mode parameter.
 */
void Metering_SetMetrologyMode(met_mode_t metrology_mode);


/**
 * @brief Deinit the HW for the specific metrology mode chosen
 * 
 * @param metrology_mode metrology mode for which to apply the deinit
 */
void Metering_MetrologyHWDeinit(met_mode_t metrology_mode);

/**
 * @brief Init the HW for the specific metrology mode chosen
 * 
 * @param metrology_mode metrology mode for which to apply the init
 */
void Metering_MetrologyHWInit(met_mode_t metrology_mode);

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

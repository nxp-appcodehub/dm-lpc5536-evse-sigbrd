/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ADC_MANAGER_H_
#define ADC_MANAGER_H_


#include "metrology_application.h"

#define ADC_INP_SAMPLES 64U /* accurate number of input samples */

/**
 * @brief Enable the Metrology flow using internal SAR ADC
 * 
 */
void ADC_Metrology_Init(void);

/**
 * @brief Stops the Metrology flow by disabling the CTIMER responsible for ADC trigger
 * 
 */
void ADC_Metrology_Deinit(void);

/**
 * @brief Enable the CP, PP ADC sampling
 * 
 */
void ADC_CP_PP_Init(void);

#endif /* ADC_MANAGER_H_ */

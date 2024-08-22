/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "swisr.h"

/******************************************************************************
 * data type definitions                                                      *
 ******************************************************************************/
static SWISR_CALLBACK pCallback0;

void NVIC_SetIsr(IRQn_Type src,uint32_t ip)
{
	NVIC_ClearPendingIRQ(src);
	NVIC_EnableIRQ(src);
	NVIC_SetPriority(src,ip);
}

void NVIC_CallIsr(IRQn_Type src)
{
	NVIC_SetPendingIRQ(src);
}

/******************************************************************************
 * public function definitions                                                *
 ******************************************************************************/
void SWISR0_HandlerInit  (uint16_t ip, SWISR_CALLBACK pCallback)
{
  if (pCallback) { pCallback0 = pCallback; NVIC_SetIsr(SWISR_VECTOR_0,ip); }
}

/******************************************************************************
 * interrupt function definitions                                             *
 ******************************************************************************/
void USB0_IRQHandler     (void) { pCallback0(); }

/******************************************************************************
 * End of module                                                              *
 ******************************************************************************/

/*
 * Copyright 2019, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "swisr.h"
#include "board.h"
#include "metrology_application.h"

/******************************************************************************
 * data type definitions                                                      *
 ******************************************************************************/
static SWISR_CALLBACK pCallback_swisr[3];

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
void ChangeIRQHandler (IRQn_Type irq, uint16_t ip, SWISR_CALLBACK pCallback)
{
	if (pCallback)
	{
		if (irq == USB0_IRQn)
		{
			pCallback_swisr[0] = pCallback;
		}
		else if (irq == USB0_NEEDCLK_IRQn) 
		{
			pCallback_swisr[1] = pCallback;
		}
		else if (irq == HWVAD0_IRQn)
		{
			pCallback_swisr[2] = pCallback;
		}
		NVIC_SetIsr(irq, ip);
	}
}
/******************************************************************************
 * interrupt function definitions                                             *
 ******************************************************************************/
void USB0_IRQHandler            (void) { pCallback_swisr[0]((void *)NULL); }
void USB0_NEEDCLK_IRQHandler    (void) { pCallback_swisr[1]((void *)kMetAFEMode); }
void HWVAD0_IRQHandler    		(void) { pCallback_swisr[2]((void *)kMetADCMode); }
/******************************************************************************
 * End of module                                                              *
 ******************************************************************************/

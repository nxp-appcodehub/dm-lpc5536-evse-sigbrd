/*
 * Copyright 2019,2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SWISR_H_
#define SWISR_H_

#include "LPC55S36.h"    /* Device header file */
#include "core_cm33.h"   /* ARM CM0+ macros and definitions */
/******************************************************************************
* Priority level definition
*
*//*! @addtogroup cm0plus_prilvl
* @{
*******************************************************************************/
#define PRI_LVL0 (uint8_t)0x00    /* < Priority level 0 (highest priority) */
#define PRI_LVL1 (uint8_t)0x01    /* < Priority level 1 */
#define PRI_LVL2 (uint8_t)0x02    /* < Priority level 2 */
#define PRI_LVL3 (uint8_t)0x03    /* < Priority level 3 */
#define PRI_LVL4 (uint8_t)0x04    /* < Priority level 4 */
/*! @} End of cm0plus_prilvl
/******************************************************************************
* List of interrupt vectors redefined for use with SWISR driver.
*
*//*! @addtogroup swisr_vectors
* @{
*******************************************************************************/
#define COMM_PROCESS_IRQ  USB0_IRQn               /* < SWISR driver handler assigned to USB0_IRQn */
#define AFE_PROCESS_IRQ   USB0_NEEDCLK_IRQn       /* < SWISR driver handler assigned to USB0_NEEDCLK_IRQn */
#define SAR_PROCESS_IRQ   HWVAD0_IRQn             /* < SWISR driver handler assigned to HWVAD0_IRQn */

/*! @} End of swisr_vectors                                                   */

/******************************************************************************
* SWISR callback registered by SWISR_HandlerInit() function
*
*//*! @addtogroup swisr_callback
* @{
*******************************************************************************/
/*! @brief SWISR_CALLBACK function declaration                                */
typedef void (*SWISR_CALLBACK)(void* met_mode);
/*! @} End of swisr_callback                                                  */

/******************************************************************************
 * public function prototypes                                                 *
 ******************************************************************************/
void NVIC_CallIsr(IRQn_Type src);
void NVIC_SetIsr(IRQn_Type src,uint32_t ip);
void ChangeIRQHandler (IRQn_Type irq, uint16_t ip, SWISR_CALLBACK pCallback);

#endif /* SWISR_H_ */

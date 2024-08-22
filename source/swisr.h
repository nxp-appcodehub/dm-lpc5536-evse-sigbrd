/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SWISR_H_
#define SWISR_H_

#include "LPC55S36.h"    /* Device header file                         */
#include "core_cm33.h"            /* ARM CM0+ macros and definitions            */
/******************************************************************************
* Priority level definition
*
*//*! @addtogroup cm0plus_prilvl
* @{
*******************************************************************************/
#define PRI_LVL0 (uint8_t)0x00    ///< Priority level 0 (highest priority)
#define PRI_LVL1 (uint8_t)0x01    ///< Priority level 1
#define PRI_LVL2 (uint8_t)0x02    ///< Priority level 2
#define PRI_LVL3 (uint8_t)0x03    ///< Priority level 3 (lowest priority)
/*! @} End of cm0plus_prilvl
/******************************************************************************
* List of interrupt vectors redefined for use with SWISR driver.
*
*//*! @addtogroup swisr_vectors
* @{
*******************************************************************************/
#define SWISR_VECTOR_0  USB0_IRQn       ///< SWISR driver handler 0 assigned to USB0_IRQn

/*! @} End of swisr_vectors                                                   */

/******************************************************************************
* SWISR callback registered by SWISR_HandlerInit() function
*
*//*! @addtogroup swisr_callback
* @{
*******************************************************************************/
/*! @brief SWISR_CALLBACK function declaration                                */
typedef void (*SWISR_CALLBACK)(void);
/*! @} End of swisr_callback                                                  */

/******************************************************************************
 * public function prototypes                                                 *
 ******************************************************************************/
void NVIC_CallIsr(IRQn_Type src);
void NVIC_SetIsr(IRQn_Type src,uint32_t ip);
extern void SWISR0_HandlerInit  (uint16_t ip, SWISR_CALLBACK pCallback);

/******************************************************************************
 * interrupt function prototypes                                              *
 ******************************************************************************/
extern void USB0_IRQHandler    (void);

#endif /* SWISR_H_ */

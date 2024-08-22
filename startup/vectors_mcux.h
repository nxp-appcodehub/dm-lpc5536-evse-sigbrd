/*
 * Copyright 2021 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VECTORS_H_
#define _VECTORS_H_

#include "LPC55S36.h"
#ifdef ENABLE_IEC60730B
#include "project_setup_lpcxpresso55s36.h"
#endif

#define WEAK __attribute__ ((weak))
//#define WEAK_AV __attribute__ ((weak, section(".after_vectors")))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

//*****************************************************************************
// Forward declaration of the core exception handlers.
// When the application defines a handler (with the same name), this will
// automatically take precedence over these weak definitions.
// If your application is a C++ one, then any interrupt handlers defined
// in C++ files within in your main application will need to have C linkage
// rather than C++ linkage. To do this, make sure that you are using extern "C"
// { .... } around the interrupt handler within your main application code.
//*****************************************************************************
     void ResetISR(void);
WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void MemManage_Handler(void);
WEAK void BusFault_Handler(void);
WEAK void UsageFault_Handler(void);
WEAK void SecureFault_Handler(void);
WEAK void SVC_Handler(void);
WEAK void DebugMon_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);
WEAK void IntDefaultHandler(void);

//*****************************************************************************
// Forward declaration of the application IRQ handlers. When the application
// defines a handler (with the same name), this will automatically take
// precedence over weak definitions below
//*****************************************************************************
WEAK void WDT_BOD_IRQHandler(void);
WEAK void DMA0_IRQHandler(void);
WEAK void GINT0_IRQHandler(void);
WEAK void GINT1_IRQHandler(void);
WEAK void PIN_INT0_IRQHandler(void);
WEAK void PIN_INT1_IRQHandler(void);
WEAK void PIN_INT2_IRQHandler(void);
WEAK void PIN_INT3_IRQHandler(void);
WEAK void UTICK0_IRQHandler(void);
WEAK void MRT0_IRQHandler(void);
WEAK void CTIMER0_IRQHandler(void);
WEAK void CTIMER1_IRQHandler(void);
WEAK void SCT0_IRQHandler(void);
WEAK void CTIMER3_IRQHandler(void);
WEAK void FLEXCOMM0_IRQHandler(void);
WEAK void FLEXCOMM1_IRQHandler(void);
WEAK void FLEXCOMM2_IRQHandler(void);
WEAK void FLEXCOMM3_IRQHandler(void);
WEAK void FLEXCOMM4_IRQHandler(void);
WEAK void FLEXCOMM5_IRQHandler(void);
WEAK void FLEXCOMM6_IRQHandler(void);
WEAK void FLEXCOMM7_IRQHandler(void);
WEAK void ADC0_IRQHandler(void);
WEAK void ADC1_IRQHandler(void);
WEAK void ACMP_IRQHandler(void);
WEAK void DMIC_IRQHandler(void);
WEAK void HWVAD0_IRQHandler(void);
WEAK void USB0_NEEDCLK_IRQHandler(void);
WEAK void USB0_IRQHandler(void);
WEAK void RTC_IRQHandler(void);
WEAK void EZH_ARCH_B0_IRQHandler(void);
WEAK void WAKEUP_IRQHandler(void);
WEAK void PIN_INT4_IRQHandler(void);
WEAK void PIN_INT5_IRQHandler(void);
WEAK void PIN_INT6_IRQHandler(void);
WEAK void PIN_INT7_IRQHandler(void);
WEAK void CTIMER2_IRQHandler(void);
WEAK void CTIMER4_IRQHandler(void);
WEAK void OS_EVENT_IRQHandler(void);
WEAK void FlexSPI0_IRQHandler(void);
WEAK void Reserved56_IRQHandler(void);
WEAK void Reserved57_IRQHandler(void);
WEAK void Reserved58_IRQHandler(void);
WEAK void CAN0_IRQ0_IRQHandler(void);
WEAK void CAN0_IRQ1_IRQHandler(void);
WEAK void SPI_FILTER_IRQHandler(void);
WEAK void Reserved62_IRQHandler(void);
WEAK void Reserved63_IRQHandler(void);
WEAK void Reserved64_IRQHandler(void);
WEAK void SEC_HYPERVISOR_CALL_IRQHandler(void);
WEAK void SEC_GPIO_INT0_IRQ0_IRQHandler(void);
WEAK void SEC_GPIO_INT0_IRQ1_IRQHandler(void);
WEAK void Freqme_IRQHandler(void);
WEAK void SEC_VIO_IRQHandler(void);
WEAK void ELS_IRQHandler(void);
WEAK void PKC_IRQHandler(void);
WEAK void PUF_IRQHandler(void);
WEAK void POWERQUAD_IRQHandler(void);
WEAK void DMA1_IRQHandler(void);
WEAK void FLEXCOMM8_IRQHandler(void);
WEAK void CDOG_IRQHandler(void);
WEAK void Reserved77_IRQHandler(void);
WEAK void I3C0_IRQHandler(void);
WEAK void Reserved79_IRQHandler(void);
WEAK void Reserved80_IRQHandler(void);
WEAK void ELS_IRQ1_IRQHandler(void);
WEAK void Tamper_IRQHandler(void);
WEAK void Reserved83_IRQHandler(void);
WEAK void Reserved84_IRQHandler(void);
WEAK void Reserved85_IRQHandler(void);
WEAK void Reserved86_IRQHandler(void);
WEAK void Reserved87_IRQHandler(void);
WEAK void Reserved88_IRQHandler(void);
WEAK void Reserved89_IRQHandler(void);
WEAK void DAC0_IRQHandler(void);
WEAK void DAC1_IRQHandler(void);
WEAK void DAC2_IRQHandler(void);
WEAK void HSCMP0_IRQHandler(void);
WEAK void HSCMP1_IRQHandler(void);
WEAK void HSCMP2_IRQHandler(void);
WEAK void FLEXPWM0_CAPTURE_IRQHandler(void);
WEAK void FLEXPWM0_FAULT_IRQHandler(void);
WEAK void FLEXPWM0_RELOAD_ERROR_IRQHandler(void);
WEAK void FLEXPWM0_COMPARE0_IRQHandler(void);
WEAK void FLEXPWM0_RELOAD0_IRQHandler(void);
WEAK void FLEXPWM0_COMPARE1_IRQHandler(void);
WEAK void FLEXPWM0_RELOAD1_IRQHandler(void);
WEAK void FLEXPWM0_COMPARE2_IRQHandler(void);
WEAK void FLEXPWM0_RELOAD2_IRQHandler(void);
WEAK void FLEXPWM0_COMPARE3_IRQHandler(void);
WEAK void FLEXPWM0_RELOAD3_IRQHandler(void);
WEAK void FLEXPWM1_CAPTURE_IRQHandler(void);
WEAK void FLEXPWM1_FAULT_IRQHandler(void);
WEAK void FLEXPWM1_RELOAD_ERROR_IRQHandler(void);
WEAK void FLEXPWM1_COMPARE0_IRQHandler(void);
WEAK void FLEXPWM1_RELOAD0_IRQHandler(void);
WEAK void FLEXPWM1_COMPARE1_IRQHandler(void);
WEAK void FLEXPWM1_RELOAD1_IRQHandler(void);
WEAK void FLEXPWM1_COMPARE2_IRQHandler(void);
WEAK void FLEXPWM1_RELOAD2_IRQHandler(void);
WEAK void FLEXPWM1_COMPARE3_IRQHandler(void);
WEAK void FLEXPWM1_RELOAD3_IRQHandler(void);
WEAK void ENC0_COMPARE_IRQHandler(void);
WEAK void ENC0_HOME_IRQHandler(void);
WEAK void ENC0_WDG_IRQHandler(void);
WEAK void ENC0_IDX_IRQHandler(void);
WEAK void ENC1_COMPARE_IRQHandler(void);
WEAK void ENC1_HOME_IRQHandler(void);
WEAK void ENC1_WDG_IRQHandler(void);
WEAK void ENC1_IDX_IRQHandler(void);
WEAK void ITRC0_IRQHandler(void);
WEAK void Reserved127_IRQHandler(void);
WEAK void ELS_ERR_IRQHandler(void);
WEAK void PKC_ERR_IRQHandler(void);
WEAK void Reserved130_IRQHandler(void);
WEAK void Reserved131_IRQHandler(void);
WEAK void Reserved132_IRQHandler(void);
WEAK void Reserved133_IRQHandler(void);
WEAK void FLASH_IRQHandler(void);
WEAK void RAM_PARITY_ECC_ERR_IRQHandler(void);

//*****************************************************************************
// Forward declaration of the driver IRQ handlers. These are aliased
// to the IntDefaultHandler, which is a 'forever' loop. When the driver
// defines a handler (with the same name), this will automatically take
// precedence over these weak definitions
//*****************************************************************************
void WDT_BOD_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DMA0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void GINT0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void GINT1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void UTICK0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void MRT0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CTIMER0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CTIMER1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SCT0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CTIMER3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM4_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM5_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM6_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM7_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ADC0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ADC1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ACMP_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DMIC_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void HWVAD0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void USB0_NEEDCLK_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void USB0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void RTC_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void EZH_ARCH_B0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void WAKEUP_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT4_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT5_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT6_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT7_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CTIMER2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CTIMER4_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void OS_EVENT_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FlexSPI0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved56_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved57_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved58_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CAN0_IRQ0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CAN0_IRQ1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SPI_FILTER_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved62_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved63_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved64_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SEC_HYPERVISOR_CALL_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SEC_GPIO_INT0_IRQ0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SEC_GPIO_INT0_IRQ1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Freqme_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void SEC_VIO_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ELS_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PKC_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PUF_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void POWERQUAD_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DMA1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXCOMM8_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void CDOG_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved77_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void I3C0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved79_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved80_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ELS_IRQ1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Tamper_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved83_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved84_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved85_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved86_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved87_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved88_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved89_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DAC0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DAC1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void DAC2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void HSCMP0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void HSCMP1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void HSCMP2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_CAPTURE_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_FAULT_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_RELOAD_ERROR_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_COMPARE0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_RELOAD0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_COMPARE1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_RELOAD1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_COMPARE2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_RELOAD2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_COMPARE3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM0_RELOAD3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_CAPTURE_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_FAULT_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_RELOAD_ERROR_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_COMPARE0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_RELOAD0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_COMPARE1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_RELOAD1_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_COMPARE2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_RELOAD2_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_COMPARE3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLEXPWM1_RELOAD3_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC0_COMPARE_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC0_HOME_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC0_WDG_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC0_IDX_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC1_COMPARE_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC1_HOME_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC1_WDG_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ENC1_IDX_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ITRC0_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved127_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void ELS_ERR_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void PKC_ERR_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved130_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved131_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved132_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void Reserved133_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void FLASH_DriverIRQHandler(void) ALIAS(IntDefaultHandler);
void RAM_PARITY_ECC_ERR_DriverIRQHandler(void) ALIAS(IntDefaultHandler);

typedef void (*vector_entry_t)(void); /* Interrupt Vector Table Function Pointers */
extern uint32_t __BOOT_STACK_ADDRESS;
extern void start(void);

#endif /* _VECTORS_H_ */

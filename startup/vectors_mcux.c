/*
 * Copyright 2021 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "vectors_mcux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*vector_entry)(void);

__attribute__((used, section(".intvec"))) const vector_entry __vector_table[] = {

(vector_entry_t)(uint32_t)& __BOOT_STACK_ADDRESS,            // The initial stack pointer
ResetISR,                          // The reset handler
NMI_Handler,                       // NMI Handler
HardFault_Handler,                 // Hard Fault Handler
MemManage_Handler,                 // MPU Fault Handler
BusFault_Handler,                  // Bus Fault Handler
UsageFault_Handler,                // Usage Fault Handler
SecureFault_Handler,               // Secure Fault Handler
0,                                 // ECRP
0,                                 // Reserved
0,                                 // Reserved
SVC_Handler,                       // SVCall Handler
DebugMon_Handler,                  // Debug Monitor Handler
0,                                 // Reserved
PendSV_Handler,                    // PendSV Handler
SysTick_Handler,                   // SysTick Handler

// Chip Level - LPC55S36
WDT_BOD_IRQHandler,                // 16 : Windowed watchdog timer, Brownout detect, Flash interrupt
DMA0_IRQHandler,                   // 17 : DMA0 controller
GINT0_IRQHandler,                  // 18 : GPIO group 0
GINT1_IRQHandler,                  // 19 : GPIO group 1
PIN_INT0_IRQHandler,               // 20 : Pin interrupt 0 or pattern match engine slice 0
PIN_INT1_IRQHandler,               // 21 : Pin interrupt 1or pattern match engine slice 1
PIN_INT2_IRQHandler,               // 22 : Pin interrupt 2 or pattern match engine slice 2
PIN_INT3_IRQHandler,               // 23 : Pin interrupt 3 or pattern match engine slice 3
UTICK0_IRQHandler,                 // 24 : Micro-tick Timer
MRT0_IRQHandler,                   // 25 : Multi-rate timer
CTIMER0_IRQHandler,                // 26 : Standard counter/timer CTIMER0
CTIMER1_IRQHandler,                // 27 : Standard counter/timer CTIMER1
SCT0_IRQHandler,                   // 28 : SCTimer/PWM
CTIMER3_IRQHandler,                // 29 : Standard counter/timer CTIMER3
FLEXCOMM0_IRQHandler,              // 30 : Flexcomm Interface 0 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM1_IRQHandler,              // 31 : Flexcomm Interface 1 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM2_IRQHandler,              // 32 : Flexcomm Interface 2 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM3_IRQHandler,              // 33 : Flexcomm Interface 3 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM4_IRQHandler,              // 34 : Flexcomm Interface 4 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM5_IRQHandler,              // 35 : Flexcomm Interface 5 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM6_IRQHandler,              // 36 : Flexcomm Interface 6 (USART, SPI, I2C, I2S, FLEXCOMM)
FLEXCOMM7_IRQHandler,              // 37 : Flexcomm Interface 7 (USART, SPI, I2C, I2S, FLEXCOMM)
ADC0_IRQHandler,                   // 38 : ADC0
ADC1_IRQHandler,                   // 39 : ADC1
ACMP_IRQHandler,                   // 40 : ACMP interrupts
DMIC_IRQHandler,                   // 41 : Digital microphone and DMIC subsystem
HWVAD0_IRQHandler,                 // 42 : Hardware Voice Activity Detector
USB0_NEEDCLK_IRQHandler,           // 43 : USB Activity Wake-up Interrupt
USB0_IRQHandler,                   // 44 : USB device
RTC_IRQHandler,                    // 45 : RTC alarm and wake-up interrupts
EZH_ARCH_B0_IRQHandler,            // 46 : EZH interrupt
WAKEUP_IRQHandler,                 // 47 : Wakeup interrupt
PIN_INT4_IRQHandler,               // 48 : Pin interrupt 4 or pattern match engine slice 4 int
PIN_INT5_IRQHandler,               // 49 : Pin interrupt 5 or pattern match engine slice 5 int
PIN_INT6_IRQHandler,               // 50 : Pin interrupt 6 or pattern match engine slice 6 int
PIN_INT7_IRQHandler,               // 51 : Pin interrupt 7 or pattern match engine slice 7 int
CTIMER2_IRQHandler,                // 52 : Standard counter/timer CTIMER2
CTIMER4_IRQHandler,                // 53 : Standard counter/timer CTIMER4
OS_EVENT_IRQHandler,               // 54 : OS_EVENT_TIMER and OS_EVENT_WAKEUP interrupts
FlexSPI0_IRQHandler,               // 55 : FlexSPI interrupt
Reserved56_IRQHandler,             // 56 : Reserved interrupt
Reserved57_IRQHandler,             // 57 : Reserved interrupt
Reserved58_IRQHandler,             // 58 : Reserved interrupt
CAN0_IRQ0_IRQHandler,              // 59 : CAN0 interrupt0
CAN0_IRQ1_IRQHandler,              // 60 : CAN0 interrupt1
SPI_FILTER_IRQHandler,             // 61 : SPI Filter interrupt
Reserved62_IRQHandler,             // 62 : Reserved interrupt
Reserved63_IRQHandler,             // 63 : Reserved interrupt
Reserved64_IRQHandler,             // 64 : Reserved interrupt
SEC_HYPERVISOR_CALL_IRQHandler,    // 65 : SEC_HYPERVISOR_CALL interrupt
SEC_GPIO_INT0_IRQ0_IRQHandler,     // 66 : SEC_GPIO_INT00 interrupt
SEC_GPIO_INT0_IRQ1_IRQHandler,     // 67 : SEC_GPIO_INT01 interrupt
Freqme_IRQHandler,                 // 68 : frequency measure interrupt
SEC_VIO_IRQHandler,                // 69 : SEC_VIO interrupt
ELS_IRQHandler,                    // 70 : ELS and ELS error interrupts
PKC_IRQHandler,                    // 71 : Public Key Cryptography interrupt
PUF_IRQHandler,                    // 72 : PUF interrupt
POWERQUAD_IRQHandler,              // 73 : PowerQuad interrupt
DMA1_IRQHandler,                   // 74 : DMA1 interrupt
FLEXCOMM8_IRQHandler,              // 75 : LSPI_HS interrupt
CDOG_IRQHandler,                   // 76 : CodeWDG interrupt
Reserved77_IRQHandler,             // 77 : Reserved interrupt
I3C0_IRQHandler,                   // 78 : I3C interrupt
Reserved79_IRQHandler,             // 79 : Reserved interrupt
Reserved80_IRQHandler,             // 80 : Reserved interrupt
ELS_IRQ1_IRQHandler,               // 81 : ELS_IRQ1
Tamper_IRQHandler,                 // 82 : Tamper
Reserved83_IRQHandler,             // 83 : Reserved interrupt
Reserved84_IRQHandler,             // 84 : Reserved interrupt
Reserved85_IRQHandler,             // 85 : Reserved interrupt
Reserved86_IRQHandler,             // 86 : Reserved interrupt
Reserved87_IRQHandler,             // 87 : Reserved interrupt
Reserved88_IRQHandler,             // 88 : Reserved interrupt
Reserved89_IRQHandler,             // 89 : Reserved interrupt
DAC0_IRQHandler,                   // 90 : dac0 interrupt
DAC1_IRQHandler,                   // 91 : dac1 interrupt
DAC2_IRQHandler,                   // 92 : dac2 interrupt
HSCMP0_IRQHandler,                 // 93 : hscmp0 interrupt
HSCMP1_IRQHandler,                 // 94 : hscmp1 interrupt
HSCMP2_IRQHandler,                 // 95 : hscmp2 interrupt
FLEXPWM0_CAPTURE_IRQHandler,       // 96 : flexpwm0_capture interrupt
FLEXPWM0_FAULT_IRQHandler,         // 97 : flexpwm0_fault interrupt
FLEXPWM0_RELOAD_ERROR_IRQHandler,  // 98 : flexpwm0_reload_error interrupt
FLEXPWM0_COMPARE0_IRQHandler,      // 99 : flexpwm0_compare0 interrupt
FLEXPWM0_RELOAD0_IRQHandler,       // 100: flexpwm0_reload0 interrupt
FLEXPWM0_COMPARE1_IRQHandler,      // 101: flexpwm0_compare1 interrupt
FLEXPWM0_RELOAD1_IRQHandler,       // 102: flexpwm0_reload1 interrupt
FLEXPWM0_COMPARE2_IRQHandler,      // 103: flexpwm0_compare2 interrupt
FLEXPWM0_RELOAD2_IRQHandler,       // 104: flexpwm0_reload2 interrupt
FLEXPWM0_COMPARE3_IRQHandler,      // 105: flexpwm0_compare3 interrupt
FLEXPWM0_RELOAD3_IRQHandler,       // 106: flexpwm0_reload3 interrupt
FLEXPWM1_CAPTURE_IRQHandler,       // 107: flexpwm1_capture interrupt
FLEXPWM1_FAULT_IRQHandler,         // 108: flexpwm1_fault interrupt
FLEXPWM1_RELOAD_ERROR_IRQHandler,  // 109: flexpwm1_reload_error interrupt
FLEXPWM1_COMPARE0_IRQHandler,      // 110: flexpwm1_compare0 interrupt
FLEXPWM1_RELOAD0_IRQHandler,       // 111: flexpwm1_reload0 interrupt
FLEXPWM1_COMPARE1_IRQHandler,      // 112: flexpwm1_compare1 interrupt
FLEXPWM1_RELOAD1_IRQHandler,       // 113: flexpwm1_reload1 interrupt
FLEXPWM1_COMPARE2_IRQHandler,      // 114: flexpwm1_compare2 interrupt
FLEXPWM1_RELOAD2_IRQHandler,       // 115: flexpwm1_reload2 interrupt
FLEXPWM1_COMPARE3_IRQHandler,      // 116: flexpwm1_compare3 interrupt
FLEXPWM1_RELOAD3_IRQHandler,       // 117: flexpwm1_reload3 interrupt
ENC0_COMPARE_IRQHandler,           // 118: enc0_compare interrupt
ENC0_HOME_IRQHandler,              // 119: enc0_home interrupt
ENC0_WDG_IRQHandler,               // 120: enc0_wdg interrupt
ENC0_IDX_IRQHandler,               // 121: enc0_idx interrupt
ENC1_COMPARE_IRQHandler,           // 122: enc1_compare interrupt
ENC1_HOME_IRQHandler,              // 123: enc1_home interrupt
ENC1_WDG_IRQHandler,               // 124: enc1_wdg interrupt
ENC1_IDX_IRQHandler,               // 125: enc1_idx interrupt
ITRC0_IRQHandler,                  // 126: itrc0 interrupt
Reserved127_IRQHandler,            // 127: Reserved interrupt
ELS_ERR_IRQHandler,                // 128: ELS error interrupt
PKC_ERR_IRQHandler,                // 129: PKC error interrupt
Reserved130_IRQHandler,            // 130: Reserved interrupt
Reserved131_IRQHandler,            // 131: Reserved interrupt
Reserved132_IRQHandler,            // 132: Reserved interrupt
Reserved133_IRQHandler,            // 133: Reserved interrupt
FLASH_IRQHandler,                  // 134: flash interrupt
RAM_PARITY_ECC_ERR_IRQHandler,     // 135: ram_parity_ecc_err interrupt
};


//*****************************************************************************
// Reset entry point for your code.
// Sets up a simple runtime environment and initializes the C/C++
// library.
//*****************************************************************************
void ResetISR(void)
{
    start();
}


WEAK void NMI_Handler(void)
{ NVIC_SystemReset();
}

WEAK void HardFault_Handler(void)
{ NVIC_SystemReset();
}

WEAK void MemManage_Handler(void)
{ NVIC_SystemReset();
}

WEAK void BusFault_Handler(void)
{ NVIC_SystemReset();
}

WEAK void UsageFault_Handler(void)
{ NVIC_SystemReset();
}

WEAK void SecureFault_Handler(void)
{ NVIC_SystemReset();
}

WEAK void SVC_Handler(void)
{ while(1) {}
}

WEAK void DebugMon_Handler(void)
{ while(1) {}
}

WEAK void PendSV_Handler(void)
{ while(1) {}
}

WEAK void SysTick_Handler(void)
{ while(1) {}
}

//*****************************************************************************
// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.
//*****************************************************************************
WEAK void IntDefaultHandler(void)
{ while(1) {}
}

//*****************************************************************************
// Default application exception handlers. Override the ones here by defining
// your own handler routines in your application code. These routines call
// driver exception handlers or IntDefaultHandler() if no driver exception
// handler is included.
//*****************************************************************************
WEAK void WDT_BOD_IRQHandler(void)
{   WDT_BOD_DriverIRQHandler();
}

WEAK void DMA0_IRQHandler(void)
{   DMA0_DriverIRQHandler();
}

WEAK void GINT0_IRQHandler(void)
{   GINT0_DriverIRQHandler();
}

WEAK void GINT1_IRQHandler(void)
{   GINT1_DriverIRQHandler();
}

WEAK void PIN_INT0_IRQHandler(void)
{   PIN_INT0_DriverIRQHandler();
}

WEAK void PIN_INT1_IRQHandler(void)
{   PIN_INT1_DriverIRQHandler();
}

WEAK void PIN_INT2_IRQHandler(void)
{   PIN_INT2_DriverIRQHandler();
}

WEAK void PIN_INT3_IRQHandler(void)
{   PIN_INT3_DriverIRQHandler();
}

WEAK void UTICK0_IRQHandler(void)
{   UTICK0_DriverIRQHandler();
}

WEAK void MRT0_IRQHandler(void)
{   MRT0_DriverIRQHandler();
}

WEAK void CTIMER0_IRQHandler(void)
{   CTIMER0_DriverIRQHandler();
}

WEAK void CTIMER1_IRQHandler(void)
{   CTIMER1_DriverIRQHandler();
}

WEAK void SCT0_IRQHandler(void)
{   SCT0_DriverIRQHandler();
}

WEAK void CTIMER3_IRQHandler(void)
{   CTIMER3_DriverIRQHandler();
}

WEAK void FLEXCOMM0_IRQHandler(void)
{   FLEXCOMM0_DriverIRQHandler();
}

WEAK void FLEXCOMM1_IRQHandler(void)
{   FLEXCOMM1_DriverIRQHandler();
}

WEAK void FLEXCOMM2_IRQHandler(void)
{   FLEXCOMM2_DriverIRQHandler();
}

WEAK void FLEXCOMM3_IRQHandler(void)
{   FLEXCOMM3_DriverIRQHandler();
}

WEAK void FLEXCOMM4_IRQHandler(void)
{   FLEXCOMM4_DriverIRQHandler();
}

WEAK void FLEXCOMM5_IRQHandler(void)
{   FLEXCOMM5_DriverIRQHandler();
}

WEAK void FLEXCOMM6_IRQHandler(void)
{   FLEXCOMM6_DriverIRQHandler();
}

WEAK void FLEXCOMM7_IRQHandler(void)
{   FLEXCOMM7_DriverIRQHandler();
}

WEAK void ADC0_IRQHandler(void)
{   ADC0_DriverIRQHandler();
}

WEAK void ADC1_IRQHandler(void)
{   ADC1_DriverIRQHandler();
}

WEAK void ACMP_IRQHandler(void)
{   ACMP_DriverIRQHandler();
}

WEAK void DMIC_IRQHandler(void)
{   DMIC_DriverIRQHandler();
}

WEAK void HWVAD0_IRQHandler(void)
{   HWVAD0_DriverIRQHandler();
}

WEAK void USB0_NEEDCLK_IRQHandler(void)
{   USB0_NEEDCLK_DriverIRQHandler();
}

WEAK void USB0_IRQHandler(void)
{   USB0_DriverIRQHandler();
}

WEAK void RTC_IRQHandler(void)
{   RTC_DriverIRQHandler();
}

WEAK void EZH_ARCH_B0_IRQHandler(void)
{   EZH_ARCH_B0_DriverIRQHandler();
}

WEAK void WAKEUP_IRQHandler(void)
{   WAKEUP_DriverIRQHandler();
}

WEAK void PIN_INT4_IRQHandler(void)
{   PIN_INT4_DriverIRQHandler();
}

WEAK void PIN_INT5_IRQHandler(void)
{   PIN_INT5_DriverIRQHandler();
}

WEAK void PIN_INT6_IRQHandler(void)
{   PIN_INT6_DriverIRQHandler();
}

WEAK void PIN_INT7_IRQHandler(void)
{   PIN_INT7_DriverIRQHandler();
}

WEAK void CTIMER2_IRQHandler(void)
{   CTIMER2_DriverIRQHandler();
}

WEAK void CTIMER4_IRQHandler(void)
{   CTIMER4_DriverIRQHandler();
}

WEAK void OS_EVENT_IRQHandler(void)
{   OS_EVENT_DriverIRQHandler();
}

WEAK void FlexSPI0_IRQHandler(void)
{   FlexSPI0_DriverIRQHandler();
}

WEAK void Reserved56_IRQHandler(void)
{   Reserved56_DriverIRQHandler();
}

WEAK void Reserved57_IRQHandler(void)
{   Reserved57_DriverIRQHandler();
}

WEAK void Reserved58_IRQHandler(void)
{   Reserved58_DriverIRQHandler();
}

WEAK void CAN0_IRQ0_IRQHandler(void)
{   CAN0_IRQ0_DriverIRQHandler();
}

WEAK void CAN0_IRQ1_IRQHandler(void)
{   CAN0_IRQ1_DriverIRQHandler();
}

WEAK void SPI_FILTER_IRQHandler(void)
{   SPI_FILTER_DriverIRQHandler();
}

WEAK void Reserved62_IRQHandler(void)
{   Reserved62_DriverIRQHandler();
}

WEAK void Reserved63_IRQHandler(void)
{   Reserved63_DriverIRQHandler();
}

WEAK void Reserved64_IRQHandler(void)
{   Reserved64_DriverIRQHandler();
}

WEAK void SEC_HYPERVISOR_CALL_IRQHandler(void)
{   SEC_HYPERVISOR_CALL_DriverIRQHandler();
}

WEAK void SEC_GPIO_INT0_IRQ0_IRQHandler(void)
{   SEC_GPIO_INT0_IRQ0_DriverIRQHandler();
}

WEAK void SEC_GPIO_INT0_IRQ1_IRQHandler(void)
{   SEC_GPIO_INT0_IRQ1_DriverIRQHandler();
}

WEAK void Freqme_IRQHandler(void)
{   Freqme_DriverIRQHandler();
}

WEAK void SEC_VIO_IRQHandler(void)
{   SEC_VIO_DriverIRQHandler();
}

WEAK void ELS_IRQHandler(void)
{   ELS_DriverIRQHandler();
}

WEAK void PKC_IRQHandler(void)
{   PKC_DriverIRQHandler();
}

WEAK void PUF_IRQHandler(void)
{   PUF_DriverIRQHandler();
}

WEAK void POWERQUAD_IRQHandler(void)
{   POWERQUAD_DriverIRQHandler();
}

WEAK void DMA1_IRQHandler(void)
{   DMA1_DriverIRQHandler();
}

WEAK void FLEXCOMM8_IRQHandler(void)
{   FLEXCOMM8_DriverIRQHandler();
}

WEAK void CDOG_IRQHandler(void)
{   CDOG_DriverIRQHandler();
}

WEAK void Reserved77_IRQHandler(void)
{   Reserved77_DriverIRQHandler();
}

WEAK void I3C0_IRQHandler(void)
{   I3C0_DriverIRQHandler();
}

WEAK void Reserved79_IRQHandler(void)
{   Reserved79_DriverIRQHandler();
}

WEAK void Reserved80_IRQHandler(void)
{   Reserved80_DriverIRQHandler();
}

WEAK void ELS_IRQ1_IRQHandler(void)
{   ELS_IRQ1_DriverIRQHandler();
}

WEAK void Tamper_IRQHandler(void)
{   Tamper_DriverIRQHandler();
}

WEAK void Reserved83_IRQHandler(void)
{   Reserved83_DriverIRQHandler();
}

WEAK void Reserved84_IRQHandler(void)
{   Reserved84_DriverIRQHandler();
}

WEAK void Reserved85_IRQHandler(void)
{   Reserved85_DriverIRQHandler();
}

WEAK void Reserved86_IRQHandler(void)
{   Reserved86_DriverIRQHandler();
}

WEAK void Reserved87_IRQHandler(void)
{   Reserved87_DriverIRQHandler();
}

WEAK void Reserved88_IRQHandler(void)
{   Reserved88_DriverIRQHandler();
}

WEAK void Reserved89_IRQHandler(void)
{   Reserved89_DriverIRQHandler();
}

WEAK void DAC0_IRQHandler(void)
{   DAC0_DriverIRQHandler();
}

WEAK void DAC1_IRQHandler(void)
{   DAC1_DriverIRQHandler();
}

WEAK void DAC2_IRQHandler(void)
{   DAC2_DriverIRQHandler();
}

WEAK void HSCMP0_IRQHandler(void)
{   HSCMP0_DriverIRQHandler();
}

WEAK void HSCMP1_IRQHandler(void)
{   HSCMP1_DriverIRQHandler();
}

WEAK void HSCMP2_IRQHandler(void)
{   HSCMP2_DriverIRQHandler();
}

WEAK void FLEXPWM0_CAPTURE_IRQHandler(void)
{   FLEXPWM0_CAPTURE_DriverIRQHandler();
}

WEAK void FLEXPWM0_FAULT_IRQHandler(void)
{   FLEXPWM0_FAULT_DriverIRQHandler();
}

WEAK void FLEXPWM0_RELOAD_ERROR_IRQHandler(void)
{   FLEXPWM0_RELOAD_ERROR_DriverIRQHandler();
}

WEAK void FLEXPWM0_COMPARE0_IRQHandler(void)
{   FLEXPWM0_COMPARE0_DriverIRQHandler();
}

WEAK void FLEXPWM0_RELOAD0_IRQHandler(void)
{   FLEXPWM0_RELOAD0_DriverIRQHandler();
}

WEAK void FLEXPWM0_COMPARE1_IRQHandler(void)
{   FLEXPWM0_COMPARE1_DriverIRQHandler();
}

WEAK void FLEXPWM0_RELOAD1_IRQHandler(void)
{   FLEXPWM0_RELOAD1_DriverIRQHandler();
}

WEAK void FLEXPWM0_COMPARE2_IRQHandler(void)
{   FLEXPWM0_COMPARE2_DriverIRQHandler();
}

WEAK void FLEXPWM0_RELOAD2_IRQHandler(void)
{   FLEXPWM0_RELOAD2_DriverIRQHandler();
}

WEAK void FLEXPWM0_COMPARE3_IRQHandler(void)
{   FLEXPWM0_COMPARE3_DriverIRQHandler();
}

WEAK void FLEXPWM0_RELOAD3_IRQHandler(void)
{   FLEXPWM0_RELOAD3_DriverIRQHandler();
}

WEAK void FLEXPWM1_CAPTURE_IRQHandler(void)
{   FLEXPWM1_CAPTURE_DriverIRQHandler();
}

WEAK void FLEXPWM1_FAULT_IRQHandler(void)
{   FLEXPWM1_FAULT_DriverIRQHandler();
}

WEAK void FLEXPWM1_RELOAD_ERROR_IRQHandler(void)
{   FLEXPWM1_RELOAD_ERROR_DriverIRQHandler();
}

WEAK void FLEXPWM1_COMPARE0_IRQHandler(void)
{   FLEXPWM1_COMPARE0_DriverIRQHandler();
}

WEAK void FLEXPWM1_RELOAD0_IRQHandler(void)
{   FLEXPWM1_RELOAD0_DriverIRQHandler();
}

WEAK void FLEXPWM1_COMPARE1_IRQHandler(void)
{   FLEXPWM1_COMPARE1_DriverIRQHandler();
}

WEAK void FLEXPWM1_RELOAD1_IRQHandler(void)
{   FLEXPWM1_RELOAD1_DriverIRQHandler();
}

WEAK void FLEXPWM1_COMPARE2_IRQHandler(void)
{   FLEXPWM1_COMPARE2_DriverIRQHandler();
}

WEAK void FLEXPWM1_RELOAD2_IRQHandler(void)
{   FLEXPWM1_RELOAD2_DriverIRQHandler();
}

WEAK void FLEXPWM1_COMPARE3_IRQHandler(void)
{   FLEXPWM1_COMPARE3_DriverIRQHandler();
}

WEAK void FLEXPWM1_RELOAD3_IRQHandler(void)
{   FLEXPWM1_RELOAD3_DriverIRQHandler();
}

WEAK void ENC0_COMPARE_IRQHandler(void)
{   ENC0_COMPARE_DriverIRQHandler();
}

WEAK void ENC0_HOME_IRQHandler(void)
{   ENC0_HOME_DriverIRQHandler();
}

WEAK void ENC0_WDG_IRQHandler(void)
{   ENC0_WDG_DriverIRQHandler();
}

WEAK void ENC0_IDX_IRQHandler(void)
{   ENC0_IDX_DriverIRQHandler();
}

WEAK void ENC1_COMPARE_IRQHandler(void)
{   ENC1_COMPARE_DriverIRQHandler();
}

WEAK void ENC1_HOME_IRQHandler(void)
{   ENC1_HOME_DriverIRQHandler();
}

WEAK void ENC1_WDG_IRQHandler(void)
{   ENC1_WDG_DriverIRQHandler();
}

WEAK void ENC1_IDX_IRQHandler(void)
{   ENC1_IDX_DriverIRQHandler();
}

WEAK void ITRC0_IRQHandler(void)
{   ITRC0_DriverIRQHandler();
}

WEAK void Reserved127_IRQHandler(void)
{   Reserved127_DriverIRQHandler();
}

WEAK void ELS_ERR_IRQHandler(void)
{   ELS_ERR_DriverIRQHandler();
}

WEAK void PKC_ERR_IRQHandler(void)
{   PKC_ERR_DriverIRQHandler();
}

WEAK void Reserved130_IRQHandler(void)
{   Reserved130_DriverIRQHandler();
}

WEAK void Reserved131_IRQHandler(void)
{   Reserved131_DriverIRQHandler();
}

WEAK void Reserved132_IRQHandler(void)
{   Reserved132_DriverIRQHandler();
}

WEAK void Reserved133_IRQHandler(void)
{   Reserved133_DriverIRQHandler();
}

WEAK void FLASH_IRQHandler(void)
{   FLASH_DriverIRQHandler();
}

WEAK void RAM_PARITY_ECC_ERR_IRQHandler(void)
{   RAM_PARITY_ECC_ERR_DriverIRQHandler();
}

/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIGBRD_APPLICATION_H__
#define SIGBRD_APPLICATION_H__

#if defined(__cplusplus)
extern "C" {
#endif
#include "types.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_lpadc.h"

#include "fsl_clock.h"
#include "fsl_inputmux.h"
#include "fsl_power.h"
#include "fsl_anactrl.h"
#include "fsl_pint.h"
#include "fsl_usart.h"
#include "fsl_ctimer.h"
#include "fsl_dac.h"

#include "control_pilot.h"
#include "proximity_pilot.h"
#include "gfci.h"
#include "relay_driver.h"
#include "comm_port_driver.h"
#include "swisr.h"

/*!
 * @addtogroup sigbrd_application
 * @{
 */

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_LPADC_BASE                  ADC0
#define DEMO_LPADC_CP_CHANNEL_1X         1U
#define DEMO_LPADC_CP_CHANNEL_3X         3U
#define DEMO_LPADC_PP_CHANNEL          	 4U
#define DEMO_LPADC_CP_CMDID            	 1U                          /* CMD1 */
#define DEMO_LPADC_VREF_SOURCE           kLPADC_ReferenceVoltageAlt3 /* VDDA */
#define DEMO_LPADC_DO_OFFSET_CALIBRATION true
#define DEMO_LPADC_USE_HIGH_RESOLUTION   true
#define DEEP_SLEEP_TIMEOUT               10000  /*10 sec, Change this value to increase or decrease the timeout counter*/
#define DEMO_UART_RX_PORT_TIMEOUT        10    /*Change this value to increase or decrease the timeout counter*/
#define HUN_MILLI_SEC                    100    /*Meterology Library to be processed after 100ms delay*/
#define TWO_SEC                          2000  /*Safety Library to be processed after 2s delay*/
#define TWO_HUND_FIFTY_MSEC              250   /*D19 Toggling Freq*/
#define ONE_HUND_TWENTYFIVE_MSEC         125   /*D18 Toggling Freq*/

/*-------  Version Numbering of the SIGBOARD code -------*/
#define MAJOR_VER               01      /* Major Release number */
#define MINOR_VER               01      /* Minor Release number*/
#define BUG_VER                 05

#define CONV_STR(s) #s
#define DEF_TO_STR(s) CONV_STR(s)

#define SIGBOARD_VERSION DEF_TO_STR(MAJOR_VER)"." DEF_TO_STR(MINOR_VER)"." DEF_TO_STR(BUG_VER)

/*SIG-BRD Hardware Version*/
#define EVSE_SIGBRD_1X 0
#define EVSE_SIGBRD_2X 2
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern volatile bool g_LpadcChnCPConversionCompletedFlag;
extern uint32_t g_LpadcChnCPConversionCompletedCount;
extern volatile bool g_LpadcChnPPConversionCompletedFlag;
extern lpadc_conv_result_t g_LpadcChnCPResultConfigStruct;
extern lpadc_conv_result_t g_LpadcChnPPResultConfigStruct;
extern kPPState g_PPState;
extern volatile kPPState g_lastPPState;
extern volatile bool g_GFCIOccurred;
extern uint32_t gfciValue;
extern lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
extern volatile float pwmOnPercent;
extern volatile uint16_t ppVal;
extern volatile uint32_t ppState;
extern volatile uint8_t onRelay;
extern volatile uint8_t offRelay;
extern volatile bool g_advertiseGFCIState;
extern volatile bool g_advertisePPState;
extern bool g_chargingCurrentState;
extern bool g_sendUpdateToMeter;
extern uint8_t g_sigboardHWVer;

/*******************************************************************************
 * Prototype
 ******************************************************************************/
/*! @brief Sleep notifications to host*/
typedef enum
{
  SLEEP_NOTIFICATION_DISABLED,
  SLEEP_NOTIFICATION_ENABLED,
  SLEEP_NOTIFICATION_SENT,
}SLEEP_NOTIFICATION_STATUS;

extern void Advertise_PP_Status(void);
extern void Advertise_GFCI_Status(void);
extern void Advertise_SleepNotificationToHost(void);
extern void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);
extern SLEEP_NOTIFICATION_STATUS g_SleepNotification;
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

#endif /* SIGBRD_APPLICATION_H__ */

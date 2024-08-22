/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "control_pilot.h"
#include "comm_port_driver.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PP_CTIMER          CTIMER0         /* Timer 0 */
#define PP_CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(0U)

#define STATEA_MIN_LEVEL				58000
#define STATEA_MAX_LEVEL				62000
#define STATEB_MIN_LEVEL				52000
#define STATEB_MAX_LEVEL				54000
#define STATEC_MIN_LEVEL				46000
#define STATEC_MAX_LEVEL				49000
#define STATEF_MIN_LEVEL				25000	/* -12V low state value */
#define STATEF_MAX_LEVEL				27000	/* -12V low state value */

/* The PWM base address */
#define BOARD_PWM_BASEADDR        PWM1
#define PWM_SRC_CLK_FREQ          CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_PWM_FAULT_LEVEL      true
#define APP_DEFAULT_PWM_FREQUENCE (1000UL)
/* Definition for default PWM frequency in Hz. */
#ifndef APP_DEFAULT_PWM_FREQUENCE
#define APP_DEFAULT_PWM_FREQUENCE (1000UL)
#endif

//#define CP_TESTMODE
#ifdef CP_TESTMODE
volatile uint8_t testMode = 1;
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Starts the PWM generation for Control Pilot.
 *
 */
void CP_StartPWM(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
kCPState g_CPState, g_OldCPState;
volatile uint8_t checkONPeriod = 1;
volatile uint16_t g_dutyCycle = 0;
uint16_t g_oldDutyCycle = 0;
bool dutyCycleChanged = false;
volatile uint32_t g_timeoutToWaitForA2ToB2 = 1000;	// SIG controller can wait for A2 to B2 transition
volatile uint32_t g_timeoutToWaitForA2ToA1 = 1000;
volatile uint32_t g_timeoutToWaitForB2ToB1 = 1000;
volatile uint32_t evDetGPIOVal = 0;
/* Structure of initialize PWM */
pwm_config_t pwmConfig;
pwm_fault_param_t faultConfig;
volatile uint32_t countRising  = 0;
volatile uint32_t countFalling  = 0;
volatile uint32_t countBothEdges = 0;
volatile uint32_t risingCaptureVal = 0;
volatile uint32_t fallingCaptureVal = 0;
volatile uint32_t ThisPeriodTmrVal, LastPeriodTmrVal, TmrPeriodCounts;
volatile uint32_t ThisOnTmrVal, TmrOnCounts;
volatile uint32_t Ctimer1Val;
volatile float pwmOnPercent = 0.0f;
volatile uint8_t checkTmrCounts = 0;
volatile uint8_t activatePilotSwitch = 0;
volatile uint8_t adcTriggerIp = 0;
volatile uint16_t skipCount;
CP_CALLBACK g_cpAppCallback;

#ifdef CP_TESTMODE
volatile bool changeDutyCycle;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initializes the Control Pilot PWM.
 *
 * @param dutyCyclePercent Percentage duty cycle of the PWM during initialization
 *
 */
static void CP_PWMInit(uint8_t dutyCyclePercent)
{
	uint16_t deadTimeVal;
	pwm_signal_param_t pwmSignal[2];
	uint32_t pwmSourceClockInHz;
	uint32_t pwmFrequencyInHz = APP_DEFAULT_PWM_FREQUENCE;

	pwmSourceClockInHz = PWM_SRC_CLK_FREQ;

	/* Set dead time count, we set this to about 650ns */
	deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

	pwmSignal[0].pwmChannel       = kPWM_PwmA;
	pwmSignal[0].level            = kPWM_HighTrue;
	pwmSignal[0].dutyCyclePercent = dutyCyclePercent;
	pwmSignal[0].deadtimeValue    = deadTimeVal;
	pwmSignal[0].faultState       = kPWM_PwmFaultState0;

	PWM_StopTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_0);
	/*********** PWMA_SM0 - phase A, configuration, setup 1 channel as an example ************/
	PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_0, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
			pwmSourceClockInHz);

	/* Setup the VAL0 trigger */
	BOARD_PWM_BASEADDR->SM[0].TCTRL |= 0x01;

	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0, true);
}

/*!
 * @brief Initializes the Control Pilot PWM.
 *
 * This function initializes the PMW at fixed 1 KHz and variable duty cycle.
 * PWM internal signal is also used to trigger the ADC channel to measure
 * Control Pilot level.
 *
 */
void CP_Init(void)
{
	ctimer_config_t config;

	/*
	 * pwmConfig.enableDebugMode = false;
	 * pwmConfig.enableWait = false;
	 * pwmConfig.reloadSelect = kPWM_LocalReload;
	 * pwmConfig.clockSource = kPWM_BusClock;
	 * pwmConfig.prescale = kPWM_Prescale_Divide_1;
	 * pwmConfig.initializationControl = kPWM_Initialize_LocalSync;
	 * pwmConfig.forceTrigger = kPWM_Force_Local;
	 * pwmConfig.reloadFrequency = kPWM_LoadEveryOportunity;
	 * pwmConfig.reloadLogic = kPWM_ReloadImmediate;
	 * pwmConfig.pairOperation = kPWM_Independent;
	 */
	PWM_GetDefaultConfig(&pwmConfig);

#ifdef DEMO_PWM_CLOCK_DEVIDER
	pwmConfig.prescale = DEMO_PWM_CLOCK_DEVIDER;
#endif

	/* Use full cycle reload */
	pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
	/* PWM A & PWM B form a complementary PWM pair */
	pwmConfig.pairOperation   = kPWM_Independent;
	pwmConfig.enableDebugMode = true;
	pwmConfig.prescale              = kPWM_Prescale_Divide_4;

	/* Initialize submodule 0 */
	if (PWM_Init(BOARD_PWM_BASEADDR, kPWM_Module_0, &pwmConfig) == kStatus_Fail)
	{
		return;
	}

	/*
	 *   config->faultClearingMode = kPWM_Automatic;
	 *   config->faultLevel = false;
	 *   config->enableCombinationalPath = true;
	 *   config->recoverMode = kPWM_NoRecovery;
	 */
	PWM_FaultDefaultConfig(&faultConfig);

#ifdef DEMO_PWM_FAULT_LEVEL
	faultConfig.faultLevel = DEMO_PWM_FAULT_LEVEL;
#endif

	/* Sets up the PWM fault protection */
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_0, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_1, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_2, &faultConfig);
	PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_3, &faultConfig);

	/* Set PWM fault disable mapping for submodule 0/1/2 */
	PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_faultchannel_0,
			kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
	PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_faultchannel_0,
			kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
	PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_faultchannel_0,
			kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);

    /* Enables the clock for the GPIO1 module */
    CLOCK_EnableClock(kCLOCK_Gpio1);

    CP_SetDutyCycle(0U);

    /* Use 12 MHz clock for some of the Ctimers */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u, true);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

    /* Initialize CTIMER for PWM period and ON time measurement */
    CTIMER_GetDefaultConfig(&config);

    /* Set pre-scale to run timer count @1MHz */
    config.prescale = (PP_CTIMER_CLK_FREQ/1000000) - 1;
    config.mode = kCTIMER_TimerMode;
    CTIMER_Init(PP_CTIMER, &config);
    /* set MAT3 to 1000 counts @1MHz clock counter = 1 msec interval */
    CTIMER_UpdatePwmPulsePeriod(PP_CTIMER, 3U, 1000);
    PP_CTIMER->EMR |= CTIMER_EMR_EMC3_MASK;
    PP_CTIMER->MCR |= CTIMER_MCR_MR3R_MASK;

    CTIMER_StartTimer(PP_CTIMER);
}

/*!
 * @brief Register callback function for Control Pilot.
 *
 * This function can be used to notify the caller/application
 * with any change indicated by CP_INFOTYPE.
 *
 * @param appCallback   Invoker callback function.
 */
void CP_RegisterAppCallback(CP_CALLBACK appCallback)
{
	g_cpAppCallback = appCallback;
}

/*!
 * @brief Starts the PWM generation for Control Pilot.
 *
 */
void CP_StartPWM(void)
{
	adcTriggerIp = 1;

	/* Start the PWM generation from Submodules 0, 1 and 2 */
	PWM_StartTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_0);
}


/*!
 * @brief Sets up the duty cycle of Control Pilot PWM.
 *
 * This function dynamically change the duty cycle of the Control Pilot PWM.
 *
 * @param dutyCyclePercentMilli   duty cycle in milli percentage. Range 0-1000.
 */
void CP_SetDutyCycle(uint16_t dutyCyclePercentMilli)
{
	/* converting 0-1000 to 0-100 range */
	CP_PWMInit(dutyCyclePercentMilli/10);
	CP_StartPWM();

	dutyCycleChanged = true;
}

/*!
 * @brief Control Process.
 *
 * This function measures the Control Pilot states and saves to user accessible variables.
 *
 */
void CP_Process(void)
{
	float temp1, temp2;
	/*
	 * PWM TRIG0[VAL0] triggers ADC at ON/2 time instance for states A,B,C,D.
	 * Occasionally, While in PC PWM state, PWM TRIG1[VAL5] can trigger ADC at OFF/2 time instance for state F.
	 */

	if (!g_LpadcChnCPConversionCompletedFlag)
	{
		return;
	}

	if (((g_LpadcChnCPResultConfigStruct.convValue) > STATEF_MIN_LEVEL) &&  (g_LpadcChnCPResultConfigStruct.convValue < STATEF_MAX_LEVEL))
	{
		g_CPState = kCPStateF;
	}
	else if ((g_LpadcChnCPResultConfigStruct.convValue > STATEA_MIN_LEVEL) &&  (g_LpadcChnCPResultConfigStruct.convValue < STATEA_MAX_LEVEL))
	{
		g_CPState = kCPStateA;
	}
	else if ((g_LpadcChnCPResultConfigStruct.convValue > STATEB_MIN_LEVEL) &&  (g_LpadcChnCPResultConfigStruct.convValue < STATEB_MAX_LEVEL))
	{
		g_CPState = kCPStateB;
	}
	else if ((g_LpadcChnCPResultConfigStruct.convValue > STATEC_MIN_LEVEL) &&  (g_LpadcChnCPResultConfigStruct.convValue < STATEC_MAX_LEVEL))
	{
		g_CPState = kCPStateC;
	}

	if(g_CPState != g_OldCPState)
	{
		if(g_cpAppCallback != NULL)
		{
			g_cpAppCallback(CP_INFOTYPE_STATE_CHANGED, g_OldCPState, g_CPState);
		}
		g_OldCPState = g_CPState;
	}

	if(dutyCycleChanged && (g_oldDutyCycle != g_dutyCycle))
	{
		if(g_cpAppCallback != NULL)
		{
			temp1 = (float)g_oldDutyCycle/10.0f;
			temp2 = (float)g_dutyCycle/10.0f;
			g_cpAppCallback(CP_INFOTYPE_DUTY_CYCLE_STATE_CHANGED, temp1, temp2);
		}
		g_oldDutyCycle = g_dutyCycle;
		dutyCycleChanged = false;
	}

#ifdef CP_TESTMODE	/* test mode */
	if(changeDutyCycle)
	{
		CP_SetDutyCycle(g_dutyCycle);
		changeDutyCycle = false;
	}
#endif
}


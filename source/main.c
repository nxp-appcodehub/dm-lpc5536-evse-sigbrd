/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "sigbrd_application.h"

#ifdef ENABLE_IEC60730B
#include "safety_application.h"
#endif

#ifdef ENABLE_METROLOGY
#include "meterology_application.h"
#endif

#ifdef LPC_TO_SJA_SPI_COMM
#include "fsl_spi.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_IRQn                  ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC      ADC0_IRQHandler

#define PP_INT_PIN_INT1_SRC              kINPUTMUX_GpioPort0Pin28ToPintsel

#define GFCI_INT_PIN_INT0_OCCURED        0
#define PP_INT_PIN_INT1_OCCURED          1


#define APP_CTIMER          CTIMER1         /* Timer 1 */
#define APP_CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(1U)
#define CTIMER_MAT_OUT kCTIMER_Match_3 /* Match output 3 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Initializes the ADC.
 *
 * This function initializes the ADC
 */
void ADC_Init(void);

/*!
 * @brief Initializes the pin used for Wakeup from Deep-Sleep Mode.
 *
 * This function initializes the GPIO_Port0_Pin28 (WAKEUP) to be used
 * as a Wakeup from Deep Sleep Mode
 */
void App_WakeUpPin_Init(void);

/*!
 * brief Interrupt callback for timer used to toggle the LEDs
 *
 * param flags
 */
void ctimer_match3_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table_app[] = {
		NULL, NULL, NULL, ctimer_match3_callback, NULL, NULL, NULL, NULL};

/*Callback function when an interrupt occurs*/
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*Variable to manage the counter for putting the sigboard into deep-sleep mode*/
uint16_t g_sleepTimeout = DEEP_SLEEP_TIMEOUT;
uint32_t g_LpadcChnCPConversionCompletedCount;
/*Variable to manage the counter for timeout the Meter RX Port if not connected*/
uint16_t g_uartRxTimeout = DEMO_UART_RX_PORT_TIMEOUT;
volatile bool g_LpadcChnCPConversionCompletedFlag = false;
lpadc_conv_result_t g_LpadcChnCPResultConfigStruct;
/*Flag managing the advertisement of PP state to host controller*/
volatile bool g_advertisePPState = false;
volatile bool g_LpadcChnPPConversionCompletedFlag = false;
volatile bool g_GFCIState;
/*Flag managing the GFCI state advertisement to Host*/
volatile bool g_advertiseGFCIState = false;
lpadc_conv_result_t g_LpadcChnPPResultConfigStruct;
lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
const uint32_t g_LpadcFullRange   = 65536U;
const uint32_t g_LpadcResultShift = 0U;
char state;
/* Match Configuration for Channel 0 */
static ctimer_match_config_t matchConfig0;
/*Excluded from entering into Deep Sleep mode*/
uint32_t g_excludeFromDS[2];
/*Wakeup pin selected for exiting from Deep Sleep*/
uint32_t g_wakeupFromDS[4];
volatile uint32_t g_LpadcInterruptCounter    = 0U;
bool g_GFCIPresent = false;
/*Variable to manage the meterology frequency*/
volatile uint32_t g_MetrologyDelayCounter = HUN_MILLI_SEC;
volatile uint32_t g_SafetyDelayCounter = TWO_SEC;
volatile bool g_SafetyLibRunTimeFlag = true;
uint8_t g_sigboardHWVer=0;


typedef enum
{
	INVALID_STATE = 0,
	CHARGING,
	NOT_CHARGING
}EVSE_CHARGE_STATE;

EVSE_CHARGE_STATE g_chargingPrevState = INVALID_STATE;
bool g_chargingCurrentState = CHARGING;
bool g_sendUpdateToMeter = false;
char *ppStates[kPPDetectedMaxVal] = {
		"\"Not detected\"",
		"\"150 Ohms detected\"",
		"\"480 Ohms detected\""
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Interrupt callback for timer used to toggle the LEDs
 *
 * param flags
 */
void ctimer_match3_callback(uint32_t flags)
{
    static uint8_t s_toggleLED = 0;

    /*Counter for managing the Meterology Processing*/
    if(g_MetrologyDelayCounter > 0)
    {
    	g_MetrologyDelayCounter--;
    }

    /*Counter for managing the Safety Library Processing*/
    if(g_SafetyDelayCounter > 0)
    {
    	g_SafetyDelayCounter--;
    }

	/*Counter managing the deep sleep mode*/
    if(g_sleepTimeout > 0)
    {
    	g_sleepTimeout--;
    }

    s_toggleLED++;
	if(g_uartRxTimeout > 0)
	{
	  g_uartRxTimeout--;
	}

	if(g_GFCIPresent)
	{
		/* APP_LED1_Blink */
		if(s_toggleLED == ONE_HUND_TWENTYFIVE_MSEC)
		{
		   GPIO_PortToggle(GPIO, BOARD_LED1_PORT, 1u << BOARD_LED1_PIN);
		}

		/*APP_LED2_Blink*/
		if(s_toggleLED == TWO_HUND_FIFTY_MSEC)
		{
			GPIO_PortToggle(GPIO, BOARD_LED2_PORT, 1u << BOARD_LED2_PIN);
            GPIO_PortToggle(GPIO, BOARD_LED1_PORT, 1u << BOARD_LED1_PIN);
            
			/*initializing to 0*/
			s_toggleLED = 0;
		}
	}
    /*LEDs toggling at half the frequency of GFCI*/
	/*LED1 is 'OFF' if no GFCI is present*/
	else if(s_toggleLED == TWO_HUND_FIFTY_MSEC)
	{
		/*Turn off the LED2*/
		GPIO_PinWrite(GPIO, BOARD_LED1_PORT, BOARD_LED1_PIN, 1u);
		GPIO_PortToggle(GPIO, BOARD_LED2_PORT, 1u << BOARD_LED2_PIN);
		/*initializing to 0*/
		s_toggleLED = 0;
	}

}

void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
	g_LpadcInterruptCounter++;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcChnCPResultConfigStruct, 0U))
#else
		if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcChnCPResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
		{
			g_LpadcChnCPConversionCompletedFlag = true;
			g_LpadcChnCPConversionCompletedCount++;
		}

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcChnPPResultConfigStruct, 1U))
#else
		if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcChnPPResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
		{
			g_LpadcChnPPConversionCompletedFlag = true;
		}

	SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Pin initialization for Wakeup from Deep-Sleep Mode
 */
void App_WakeUpPin_Init(void)
{
	/* Configure the Input Mux block and connect the trigger source to PinInt channel. */
	INPUTMUX_Init(INPUTMUX);
	INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt1, PP_INT_PIN_INT1_SRC); /* Using channel 1. */

	/* Configure the interrupt for SW pin. */
	PINT_Init(PINT);
	PINT_PinInterruptConfig(PINT, kPINT_PinInt1, kPINT_PinIntEnableFallEdge, pint_intr_callback);
	PINT_EnableCallback(PINT); /* Enable callbacks for PINT */
}

/*!
 * @brief LED initialization function
 */
void APP_LED_Init(void)
{
	ctimer_config_t config;

	/* Use 12 MHz clock for some of the Ctimers */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 0u, false);
	CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 1u, true);
	CLOCK_AttachClk(kFRO_HF_to_CTIMER1);

	/* Initialize CTIMER for PWM period and ON time measurement */
	CTIMER_GetDefaultConfig(&config);

	/* Set pre-scale to run timer count @1MHz */
	config.prescale = (APP_CTIMER_CLK_FREQ/1000000) - 1;
	config.mode = kCTIMER_TimerMode;
	CTIMER_Init(APP_CTIMER, &config);

	/* Configuration 0 */
	matchConfig0.enableCounterReset = true;
	matchConfig0.enableCounterStop  = false;
	matchConfig0.matchValue         = 1000; /*changed to 1ms*/
	matchConfig0.outControl         = kCTIMER_Output_Toggle;
	matchConfig0.outPinInitState    = false;
	matchConfig0.enableInterrupt    = true;

	CTIMER_RegisterCallBack(APP_CTIMER, &ctimer_callback_table_app[0], kCTIMER_MultipleCallback);
	CTIMER_SetupMatch(APP_CTIMER, CTIMER_MAT_OUT, &matchConfig0);
	CTIMER_StartTimer(APP_CTIMER);
}

/*!
 * @brief ADC initialization function
 */
void ADC_Init(void)
{
	lpadc_config_t mLpadcConfigStruct;
	lpadc_conv_command_config_t mLpadcCommandConfigStruct;

	/* setup ADC channels for PP and CP measurement */
	LPADC_GetDefaultConfig(&mLpadcConfigStruct);
	mLpadcConfigStruct.enableAnalogPreliminary = true;
#if defined(DEMO_LPADC_VREF_SOURCE)
	mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
	mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
	LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
	/* Request offset calibration. */
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
	LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
#else
	LPADC_SetOffsetValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
	/* Request gain calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
	/* Do auto calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

	/* Set conversion CMD configuration for CP. */
	LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);

	if(g_sigboardHWVer == EVSE_SIGBRD_1X)
	{
		mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_CP_CHANNEL_1X;
	}

	else if(g_sigboardHWVer == EVSE_SIGBRD_2X)
	{
		mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_CP_CHANNEL_3X;
	}

#if defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION
	mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelDualSingleEndBothSide;
	LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_CP_CMDID, &mLpadcCommandConfigStruct);
	/* select alt en chan 4B - Proximity Pilot PP */
	if(g_sigboardHWVer == EVSE_SIGBRD_1X)
	{
		ADC0->CMD[DEMO_LPADC_CP_CMDID-1].CMDL |= ADC_CMDL_ALTBEN(DEMO_LPADC_CP_CHANNEL_1X) | ADC_CMDL_ALTB_ADCH(DEMO_LPADC_PP_CHANNEL);
	}

	else if(g_sigboardHWVer == EVSE_SIGBRD_2X)
	{
		ADC0->CMD[DEMO_LPADC_CP_CMDID-1].CMDL |= ADC_CMDL_ALTBEN(DEMO_LPADC_CP_CHANNEL_3X) | ADC_CMDL_ALTB_ADCH(DEMO_LPADC_PP_CHANNEL);
	}


	/* Set trigger configuration for CP. */
	LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
	mLpadcTriggerConfigStruct.enableHardwareTrigger = true;
	mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_CP_CMDID;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
	mLpadcTriggerConfigStruct.channelAFIFOSelect = 0U;
	mLpadcTriggerConfigStruct.channelBFIFOSelect = 1U;
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
	LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

	/* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
	LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
	NVIC_SetPriority(DEMO_LPADC_IRQn, 1);
	EnableIRQ(DEMO_LPADC_IRQn);
}

/*!
 * @brief Call back for PINT Pin interrupt 0-7.
 *
 * GFCI - Port 1 Pin 30, Proximity Pilot - Port 0, Pin 28 interrupts
 * @param pintr interrupt pin for which interrupt occurs             .
 * @param pmatch_status
 */
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{

	if(pintr == GFCI_INT_PIN_INT0_OCCURED)
	{
		/* GFCI interrupt */
		gfciValue = GPIO_PinRead(GPIO, BOARD_GFCI_INT_PORT, BOARD_GFCI_INT_PIN);
		g_GFCIOccurred = true;
		/* Advertise GFCI status */
		g_advertiseGFCIState = true;
	}

	else if(pintr == PP_INT_PIN_INT1_OCCURED)
	{
		/* Advertise PP status */
		g_advertisePPState = true;
		/* Update the Deep Sleep timer*/
		g_sleepTimeout = DEEP_SLEEP_TIMEOUT;
	}

}

/*!
 * @brief Pointing to function CP_RegisterAppCallback.
 *
 * It can be used to notify the caller/application
 * with any change indicated by CP_INFOTYPE.
 *
 * @param kInfoType Info types for application callback function.
 * @param param1    1st parameter.
 * @param param2    2nd parameter.
 */
static void App_CP_Callback(CP_INFOTYPE kInfoType, uint32_t param1, uint32_t param2)
{
	switch (kInfoType)
	{
	case CP_INFOTYPE_STATE_CHANGED:
	     if((param2 == kCPStateC) || (param2 == kCPStateD))
	     {
	    	 g_chargingCurrentState = true;
	     }

	     else
	     {
	    	 g_chargingCurrentState = false;
	     }
		break;
	case CP_INFOTYPE_DUTY_CYCLE_STATE_CHANGED:
		/* Add your handler here */
		break;
	default:
		break;
	}
}

/*!
 * @brief This function is pointing to PP_RegisterAppCallback.
 *
 * It can be used to notify the caller/application
 * with any change indicated by PP_INFOTYPE.
 *
 * @param kInfoType Info types for application callback function.
 * @param param1    1st parameter.
 * @param param2    2nd parameter.
 */
static void App_PP_Callback(PP_INFOTYPE kInfoType, uint32_t param1, uint32_t param2)
{
	switch (kInfoType)
	{
	case PP_INFOTYPE_STATE_CHANGED:
		if(param2 == kPPNotDetected)
		{
			g_chargingCurrentState = false;
		}

		else
		{
			g_chargingCurrentState = true;
		}
		break;
	default:
		break;
	}
}

/*!
 * @brief Pointing to function GFCI_RegisterAppCallback
 *
 * It can be used to notify the caller/application
 * with any change indicated by GFCI_INFOTYPE.
 *
 * @param1 kInfoType Info types for application callback function.
 * @param2 param     GPIO Pin Value.
 */
static void App_GFCI_Callback(GFCI_INFOTYPE kInfoType, uint32_t param)
{
	switch (kInfoType)
	{
	case GFCI_INFOTYPE_STATE_CHANGED:
		if(param == 1U)
		{
			/*GFCI present */
			g_GFCIPresent = true;
			g_chargingCurrentState = false;
		}
		else
		{
			/*GFCI not present */
			g_GFCIPresent = false;
			g_chargingCurrentState = true;
		}
		break;
	default:
		break;
	}
}

/*!
 * @brief Pointing to function Relay_RegisterAppCallback
 *
 * It can be used to notify the caller/application
 * with any change indicated by RELAY_INFOTYPE.
 *
 * @param kInfoType Info types for application callback function.
 * @param param    1st parameter.
 */
static void App_Relay_Callback(RELAY_INFOTYPE kInfoType, uint32_t param)
{
	switch (kInfoType)
	{
	case RELAY_INFOTYPE_STATE_CHANGED:
	    if(param == 1U)
	    {
	    	g_chargingCurrentState = true;
	    }

	    else
	    {
	    	g_chargingCurrentState = false;
	    }


		break;

	case RELAY_INFOTYPE_EM_STOP:
		/* Add your handler here */
		break;
	default:
		break;
	}
}


/*!
 * @brief Main function
 */
int main(void)
{
	/* Board pin, clock, debug console init */
	/* attach 12 MHz clock to FLEXCOMM0 (debug console) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
	/* attach 12 MHz clock to FLEXCOMM3 (LIN slave) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 0u, false);
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1u, true);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);
	/* attach 12 MHz clock to FLEXCOMM1 (M_UART) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 0u, false);
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 1u, true);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);
#ifdef LPC_TO_SJA_SPI_COMM
	/* attach 12 MHz clock to SPI2 */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 0u, false);
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u, true);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
#endif

	RESET_ClearPeripheralReset(kFC0_RST_SHIFT_RSTn);

	/* Init output HPGP_RESET GPIO. */
	GPIO_PortInit(GPIO, 0U);
	GPIO_PortInit(GPIO, 1U);

#ifdef LPC_TO_SJA_SPI_COMM
	/* reset FLEXCOMM for SPI */
	RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
#endif


    /*Checking the pin status for getting the hardware version*/
	g_sigboardHWVer = BOARD_PinStatus();
	BOARD_InitPins();
	BOARD_BootClockPLL150M();
	BOARD_InitDebugConsole();


    /* Analog components excluded from Deep Sleep mode*/
    g_excludeFromDS[0]  = kPDRUNCFG_PD_DCDC | kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K;
    g_excludeFromDS[1]  = 0;

    /* Wakeup from Deep Sleep through PP_WAKEUP, host UART(Flexcomm 0) */
    g_wakeupFromDS[0]  = WAKEUP_GPIO_INT0_1 | WAKEUP_FLEXCOMM0;
    g_wakeupFromDS[1]  = 0;
    g_wakeupFromDS[2]  = 0;
    g_wakeupFromDS[3]  = 0;

    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 2U, true);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);

    /* Disable VREF power down */
    POWER_DisablePD(kPDRUNCFG_PD_VREF);

    /* initialize PP - Port 0,Pin 28 (PP_WAKEUP)to Wakeup from Sleep Mode */
    App_WakeUpPin_Init();

    /* initialize the board uart modules */
    Uart_ModuleInit();

    ANACTRL_Init(ANACTRL);
    ANACTRL_EnableVref1V(ANACTRL, true);

    /*Initialize the ADC*/
    ADC_Init();

    SYSCON->PWM1SUBCTL |=
    		(SYSCON_PWM1SUBCTL_CLK0_EN_MASK | SYSCON_PWM1SUBCTL_CLK1_EN_MASK | SYSCON_PWM1SUBCTL_CLK2_EN_MASK);

    /* initialize Control Pilot */
    CP_Init();
    CP_RegisterAppCallback(App_CP_Callback);

    /* initialize Proximity Pilot */
    PP_Init();
    PP_RegisterAppCallback(App_PP_Callback);

    /* initialize GFCI detection h/w logic */
    GFCI_Init();
    GFCI_RegisterAppCallback(App_GFCI_Callback);

    /* Initialize relay */
    Relay_Init();
    Relay_RegisterAppCallback(App_Relay_Callback);

    /* Initialize for APP default task/LED toggle */
    APP_LED_Init();

    /* Initialize the Software interrupt for Comm_Process*/
    SWISR0_HandlerInit (PRI_LVL2, Comm_Process);

#ifdef ENABLE_IEC60730B
	safetyTest_afterReset();
#endif

#ifdef ENABLE_METROLOGY
	/*Initialize the metering APIs*/
	Metering_Init();
#endif

	/* Initialization done */
	/* Entering main loop */
	while (1U)
	{

#ifdef ENABLE_IEC60730B

		if(g_SafetyDelayCounter == 0)
		{
		  safetyTest_runTime();
		  g_SafetyDelayCounter = TWO_SEC;
		}
#endif

		/* PP_Process detects Proximity Pilot detection /w load current capacity */
		PP_Process();

		/* CP_Process executes host controller commands and switch PWM state */
		CP_Process();

		/* handling advertisements/notifications */
		Comm_Process_Advertisement();

		/* GFCI_Process executes host controller commands and switch PWM state */
		GFCI_Process();

		/* Check relay state */
		Relay_Process();

		/*If Current and Previous state of SIGBOARD is not same, send the notification to Meter*/
		if(g_chargingCurrentState != g_chargingPrevState)
		{
			/*Flag for enabling the notification, message will be sent only once*/
			g_sendUpdateToMeter = true;
			g_chargingPrevState = g_chargingCurrentState;
		}
		/* EVSE Sigboard needs to send the notification to host before entering into low power mode,
		 * only when UART Rx, Tx port are in idle state for time DEEP_SLEEP_TIMEOUT. And after sending
		 * the notification it will again check the status of UART Rx, Tx ports (Idle state) to confirm whether all
		 * the ongoing transmission is done or not. g_SleepNotification is used for implementing the same.
		 */

		if(g_SleepNotification == SLEEP_NOTIFICATION_SENT)
		{
			g_sleepTimeout = DEEP_SLEEP_TIMEOUT;
			g_SleepNotification = SLEEP_NOTIFICATION_DISABLED;
			g_uartRxTimeout = DEMO_UART_RX_PORT_TIMEOUT;
			g_MetrologyDelayCounter = HUN_MILLI_SEC;
			g_SafetyDelayCounter = TWO_SEC;
			g_SafetyLibRunTimeFlag = false;

			/*Turn off the LED1 & LED2 before entering into sleep mode*/
		    GPIO_PinWrite(GPIO, BOARD_LED1_PORT, BOARD_LED1_PIN, 1u);
		    GPIO_PinWrite(GPIO, BOARD_LED2_PORT, BOARD_LED2_PIN, 1u);

			/*Entering the device into Deep-Sleep mode*/
			POWER_EnterDeepSleep(g_excludeFromDS, 0x0, g_wakeupFromDS, 0x0);
		}

#ifdef ENABLE_METROLOGY
		if(g_MetrologyDelayCounter == 0)
		{
			/*Process the Metering Data*/
			Metering_Process();
			g_MetrologyDelayCounter = HUN_MILLI_SEC;
		}
#endif

	}

}

/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dma.h"
#include "fsl_inputmux.h"
#include "fsl_vref.h"
#include "fsl_opamp.h"
#include "sigbrd_application.h"
#include "adc_manager.h"
#include <metrology_application.h>
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* DMA buffers defines*/
#define ADC_NO_DATA_WORDS               3U

#define DEMO_LPADC_IRQn                 ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC     ADC0_IRQHandler

#define DEMO_LPADC0_RESFIFO_REG_ADDR    (uint32_t)(&(ADC0->RESFIFO[0]))
#define DEMO_LPADC1_RESFIFO_REG_ADDR    (uint32_t)(&(ADC1->RESFIFO[0]))
#define DEMO_DMA_BASE                   DMA0
#define DEMO_DMA_TRANSFER_TYPE          kDMA_PeripheralToMemory
#define DEMO_DMA_ADC1_CHANNEL  			27U
#define DEMO_DMA_ADC0_CHANNEL           21U
#define DMA_DESCRIPTOR_NUM              4U

#define DEMO_ADC_SAMPLE_COUNT          	ADC_INP_SAMPLES

#define DEMO_SAMPLE_COUNT_SECOND 		(DEMO_DISTRIBUTION_FREQUENCY * DEMO_ADC_SAMPLE_COUNT)
#define DEMO_CADC_SAMPLE_COUNT 			(DEMO_ADC_SAMPLE_COUNT * DEMO_PHASE) /* The lpadc sample count. */

#define DEMO_TOTAL_SAMPLE_COUNT 		(DEMO_CADC_SAMPLE_COUNT)

#define ADC_DMA_BUFFERS			        2U

enum _adc_params_t
{
	ADC_U1_INDEX,
	ADC_U2_INDEX,
	ADC_U3_INDEX,
	ADC_I1_INDEX,
	ADC_I2_INDEX,
	ADC_I3_INDEX,
	ADC_PARAMETER_COUNT
};
/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t g_XferConfig_21;
uint32_t g_XferConfig_27;

dma_handle_t g_DmaHandleStruct_21;            /* Handler structure for using DMA. */
dma_handle_t g_DmaHandleStruct_27;            /* Handler structure for using DMA. */

/* Keep the ADC conversion result moved from ADC data register by DMA. */
uint32_t g_Adc0ConvResult[ADC_DMA_BUFFERS][DEMO_ADC_SAMPLE_COUNT * ADC_NO_DATA_WORDS];

uint32_t g_Adc1ConvResult[ADC_DMA_BUFFERS][DEMO_ADC_SAMPLE_COUNT * ADC_NO_DATA_WORDS];

/* DMA descriptor table used for ping-pong mode. */
SDK_ALIGN(uint32_t s_dma_table[DMA_DESCRIPTOR_NUM * sizeof(dma_descriptor_t)],
          FSL_FEATURE_DMA_LINK_DESCRIPTOR_ALIGN_SIZE);

volatile bool g_DmaTransferDoneFlag = false; /* Flag of DMA transfer done trigger by ADC1 conversion. */
/*******************************************************************************
 * Code
 ******************************************************************************/

static void ClockInit(void)
{
    /* Enable clock for ADC0 */
    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 2U, true);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);
    CLOCK_EnableClock(kCLOCK_Adc0);

	/* Enable clock for ADC1 */
	CLOCK_SetClkDiv(kCLOCK_DivAdc1Clk, 2U, true);
	CLOCK_AttachClk(kFRO_HF_to_ADC1);
	CLOCK_EnableClock(kCLOCK_Adc1);

	CLOCK_EnableClock(kCLOCK_Dma0);

}

static void VREF_Configuration(void)
{
    vref_config_t vrefConfig;

    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(VREF, &vrefConfig);
}

static void OPAMP_Configuration(void)
{
    opamp_config_t config;
    /*
     *  config->enable        = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *
     *  config->enablePosADCSw = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvert16X;
    config.negGain = kOPAMP_NegGainInvert16X;
    /* Connect REFP to VREF output. */
    config.posRefVoltage = kOPAMP_PosRefVoltVrefh1;
    config.enable        = true;
    OPAMP_Init(OPAMP0, &config);
    OPAMP_Init(OPAMP1, &config);
    OPAMP_Init(OPAMP2, &config);
}
/* Used only for performance measurement */
static void CTIMER0_Init(void)
{
	ctimer_match_config_t matchConfig;
	ctimer_config_t config;

	/* CTIMER 0 clock divider enable */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u, true);
	CLOCK_AttachClk(kFRO1M_to_CTIMER0);
	CLOCK_EnableClock(kCLOCK_Timer0);


	/* Initialize CTIMER for PWM period and ON time measurement */
	CTIMER_GetDefaultConfig(&config);

	config.prescale = 0;
	config.mode = kCTIMER_TimerMode;
	CTIMER_Init(CTIMER0, &config);

	uint32_t freq = CLOCK_GetCTimerClkFreq(0);

	matchConfig.enableCounterReset = true;
	matchConfig.enableCounterStop  = false;
	matchConfig.matchValue         = freq * 10;
	matchConfig.outControl         = kCTIMER_Output_Toggle;
	matchConfig.outPinInitState    = false;
	matchConfig.enableInterrupt    = false;

	CTIMER_SetupMatch(CTIMER0, 0, &matchConfig);
	CTIMER_StartTimer(CTIMER0);

}

/**
 * @brief Init the CTIMER4 freq to match with DEMO_SAMPLE_COUNT_SECOND define.
 * 
 */
static void CTIMER4_Init(void)
{
	ctimer_match_config_t matchConfig;
	ctimer_config_t config;

	/* CTIMER 4 clock divider enable */
	CLOCK_SetClkDiv(kCLOCK_DivCtimer4Clk, 1u, true);
	CLOCK_AttachClk(kPLL0_to_CTIMER4);
	CLOCK_EnableClock(kCLOCK_Timer4);


	/* Initialize CTIMER for PWM period and ON time measurement */
	CTIMER_GetDefaultConfig(&config);

	config.prescale = 0;
	config.mode = kCTIMER_TimerMode;
	CTIMER_Init(CTIMER4, &config);

	uint32_t freq = CLOCK_GetCTimerClkFreq(4);
	freq =  freq / DEMO_SAMPLE_COUNT_SECOND / 2 / (config.prescale + 1);


	matchConfig.enableCounterReset = true;
	matchConfig.enableCounterStop  = false;
	matchConfig.matchValue         = freq;
	matchConfig.outControl         = kCTIMER_Output_Toggle;
	matchConfig.outPinInitState    = false;
	matchConfig.enableInterrupt    = false;

	CTIMER_SetupMatch(CTIMER4, 0, &matchConfig);
	matchConfig.enableCounterReset = false;
	matchConfig.matchValue         = freq - 1;
	CTIMER_SetupMatch(CTIMER4, 1, &matchConfig);
	CTIMER_SetupMatch(CTIMER4, 3, &matchConfig);

	CTIMER_StartTimer(CTIMER4);

}

/**
 * @brief Stop the CTIMER4
 * 
 */
static void CTIMER4_Deinit(void)
{
	CTIMER_StopTimer(CTIMER4);
}


void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	if (LPADC_GetConvResult(DEMO_LPADC_CP_BASE, &g_LpadcChnCPResultConfigStruct, 1U))
#else
		if (LPADC_GetConvResult(DEMO_LPADC_CP_BASE, &g_LpadcChnCPResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
		{
			g_LpadcChnCPConversionCompletedFlag = true;
			g_LpadcChnCPConversionCompletedCount++;
		}

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	if (LPADC_GetConvResult(DEMO_LPADC_CP_BASE, &g_LpadcChnPPResultConfigStruct, 1U))
#else
		if (LPADC_GetConvResult(DEMO_LPADC_CP_BASE, &g_LpadcChnPPResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
		{
			g_LpadcChnPPConversionCompletedFlag = true;
		}

	if (DEMO_LPADC_CP_BASE->STAT & ADC_STAT_FOF1_MASK)
	{
		LPADC_DoResetFIFO1(DEMO_LPADC_CP_BASE);
		LPADC_ClearStatusFlags(DEMO_LPADC_CP_BASE, kLPADC_ResultFIFO1OverflowFlag);
	}

	SDK_ISR_EXIT_BARRIER;
}

/**
 * @brief Init the ADC0
 * 
 */
static void ADC0_Init()
{
	lpadc_config_t mLpadcConfigStruct;

	/* setup ADC channels for PP and CP measurement */
	LPADC_GetDefaultConfig(&mLpadcConfigStruct);
	mLpadcConfigStruct.enableAnalogPreliminary = true;
	mLpadcConfigStruct.FIFO0Watermark = 2;
	mLpadcConfigStruct.FIFO1Watermark = 1 ; /* One from PP and one from CP */

#if defined(DEMO_LPADC_VREF_SOURCE)
	mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
	mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
	LPADC_Init(DEMO_LPADC_CP_BASE, &mLpadcConfigStruct);

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
	/* Request offset calibration. */
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
	LPADC_DoOffsetCalibration(DEMO_LPADC_CP_BASE);
#else
	LPADC_SetOffsetValue(DEMO_LPADC_CP_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
	/* Request gain calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_CP_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
	/* Do auto calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_CP_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

}

static void ADC0_CP_PP_Configure(void)
{

	lpadc_conv_command_config_t mLpadcCommandConfigStruct;
	lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;

    /***************************************************************************/
    /**** CMD PP and CP                                              		****/
    /***************************************************************************/
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
	mLpadcCommandConfigStruct.enableWaitTrigger = 0;
	mLpadcCommandConfigStruct.enableChannelB    = true;
	mLpadcCommandConfigStruct.channelBNumber    = DEMO_LPADC_PP_CHANNEL;
	LPADC_SetConvCommandConfig(DEMO_LPADC_CP_BASE, DEMO_LPADC_CP_CMDID, &mLpadcCommandConfigStruct);

    /***************************************************************************/
    /**** Trigger CP                                                 		****/
    /***************************************************************************/
	LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
	mLpadcTriggerConfigStruct.enableHardwareTrigger = true;
	mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_CP_CMDID;

	mLpadcTriggerConfigStruct.priority = 0;

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
	mLpadcTriggerConfigStruct.channelAFIFOSelect = 1U;
	mLpadcTriggerConfigStruct.channelBFIFOSelect = 1U;
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */

	LPADC_SetConvTriggerConfig(DEMO_LPADC_CP_BASE, DEMO_LPADC_CP_TRIGER_ID, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

	INPUTMUX_AttachSignal(INPUTMUX, DEMO_LPADC_CP_TRIGER_ID, kINPUTMUX_Pwm1Sm0MuxTrig0ToAdc0Trigger);

	/* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
	LPADC_EnableInterrupts(DEMO_LPADC_CP_BASE, kLPADC_FIFO1WatermarkInterruptEnable);
#else
	LPADC_EnableInterrupts(DEMO_LPADC_CP_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */

	NVIC_SetPriority(DEMO_LPADC_IRQn, 1);
	EnableIRQ(DEMO_LPADC_IRQn);

}

/**
 * @brief Configure ADC0 for current sampling. Sets the trigger to start the ADC sampling and enable DMA flag assert
 * 
 */
static void ADC0_Current_Configure(void)
{

	lpadc_config_t mLpadcConfigStruct;
	lpadc_conv_command_config_t mLpadcCommandConfigStruct;
	lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;

    /***************************************************************************/
    /**** CMD Current                                              		    ****/
    /***************************************************************************/

	LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
	mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
	mLpadcCommandConfigStruct.channelNumber 	= DEMO_LPADC_I1_CHANNEL;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
	mLpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount32;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = DEMO_LPADC_I2_CMDID;

	LPADC_SetConvCommandConfig(DEMO_LPADC_CURRENT_BASE, DEMO_LPADC_I1_CMDID, &mLpadcCommandConfigStruct);

	mLpadcCommandConfigStruct.enableWaitTrigger =  false;
	mLpadcCommandConfigStruct.channelNumber 	= 0;
	mLpadcCommandConfigStruct.enableChannelB 	= true;
	mLpadcCommandConfigStruct.channelBNumber    = DEMO_LPADC_I2_CHANNEL;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = DEMO_LPADC_I3_CMDID;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideB;

	LPADC_SetConvCommandConfig(DEMO_LPADC_CURRENT_BASE, DEMO_LPADC_I2_CMDID, &mLpadcCommandConfigStruct);

	mLpadcCommandConfigStruct.channelNumber 	= DEMO_LPADC_I3_CHANNEL;
	mLpadcCommandConfigStruct.enableChannelB 	= false;
	mLpadcCommandConfigStruct.channelBNumber    = 0;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = 0;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;

	LPADC_SetConvCommandConfig(DEMO_LPADC_CURRENT_BASE, DEMO_LPADC_I3_CMDID, &mLpadcCommandConfigStruct);

    /***************************************************************************/
    /**** Trigger Current                                              		****/
    /***************************************************************************/
	LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
	mLpadcTriggerConfigStruct.enableHardwareTrigger = true;
	mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_I1_CMDID;
	mLpadcTriggerConfigStruct.priority  = 1;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
	mLpadcTriggerConfigStruct.channelAFIFOSelect = 0U;
	mLpadcTriggerConfigStruct.channelBFIFOSelect = 0U;
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
	LPADC_SetConvTriggerConfig(DEMO_LPADC_CURRENT_BASE, DEMO_LPADC_CURRENT_TRIGER_ID, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

	 /* ADC0 trigger "1" select, CTIMR4 MAT3 */
	INPUTMUX_AttachSignal(INPUTMUX, DEMO_LPADC_CURRENT_TRIGER_ID, kINPUTMUX_Ctimer4M3ToAdc0Trigger);

	LPADC_EnableFIFO0WatermarkDMA(DEMO_LPADC_CURRENT_BASE, true);
}

/**
 * @brief Init the ADC1 
 * 
 */
static void ADC1_Init(void)
{
	lpadc_config_t mLpadcConfigStruct;

	LPADC_GetDefaultConfig(&mLpadcConfigStruct);
	mLpadcConfigStruct.enableAnalogPreliminary = true;
	mLpadcConfigStruct.FIFO0Watermark = 2;

#if defined(DEMO_LPADC_VREF_SOURCE)
	mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
	mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
	LPADC_Init(DEMO_LPADC_VOLTAGE_BASE, &mLpadcConfigStruct);

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
	/* Request offset calibration. */
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
	LPADC_DoOffsetCalibration(DEMO_LPADC_VOLTAGE_BASE);
#else
	LPADC_SetOffsetValue(DEMO_LPADC_VOLTAGE_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
	/* Request gain calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_VOLTAGE_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
	/* Do auto calibration. */
	LPADC_DoAutoCalibration(DEMO_LPADC_VOLTAGE_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

}

/**
 * @brief Configure ADC! for voltage sampling. Sets the trigger to start the ADC sampling and enable DMA flag assert
 * 
 */
void ADC1_Voltage_Configure(void)
{

	lpadc_conv_command_config_t mLpadcCommandConfigStruct;
	lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;

    /***************************************************************************/
    /**** CMD VOLTAGE                                              		    ****/
    /***************************************************************************/

	LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
	mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
	mLpadcCommandConfigStruct.channelNumber 	= DEMO_LPADC_U1_CHANNEL;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
	mLpadcCommandConfigStruct.enableWaitTrigger = true;
	mLpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount32;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = DEMO_LPADC_U2_CMDID;

	LPADC_SetConvCommandConfig(DEMO_LPADC_VOLTAGE_BASE, DEMO_LPADC_U1_CMDID, &mLpadcCommandConfigStruct);

	mLpadcCommandConfigStruct.enableWaitTrigger =  false;
	mLpadcCommandConfigStruct.channelNumber 	= DEMO_LPADC_U2_CHANNEL;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = DEMO_LPADC_U3_CMDID;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;

	LPADC_SetConvCommandConfig(DEMO_LPADC_VOLTAGE_BASE, DEMO_LPADC_U2_CMDID, &mLpadcCommandConfigStruct);

	mLpadcCommandConfigStruct.channelNumber 	= DEMO_LPADC_U3_CHANNEL;
	mLpadcCommandConfigStruct.chainedNextCommandNumber = 0;
	mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;

	LPADC_SetConvCommandConfig(DEMO_LPADC_VOLTAGE_BASE, DEMO_LPADC_U3_CMDID, &mLpadcCommandConfigStruct);

    /***************************************************************************/
    /**** Trigger Voltage                                              	    ****/
    /***************************************************************************/
	LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
	mLpadcTriggerConfigStruct.enableHardwareTrigger = true;
	mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_U1_CMDID;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
	mLpadcTriggerConfigStruct.channelAFIFOSelect = 0U;
	mLpadcTriggerConfigStruct.channelBFIFOSelect = 0U;
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
	LPADC_SetConvTriggerConfig(DEMO_LPADC_VOLTAGE_BASE, DEMO_LPADC_VOLTAGE_TRIGER_ID, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

	 /* ADC1 trigger "0" select, CTIMR4 MAT1 */
	INPUTMUX_AttachSignal(INPUTMUX, DEMO_LPADC_VOLTAGE_TRIGER_ID, kINPUTMUX_Ctimer4M1ToAdc1Trigger);

	LPADC_EnableFIFO0WatermarkDMA(DEMO_LPADC_VOLTAGE_BASE, true);

}

/* Software ISR for DMA transfer done. */
void DEMO_DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	static uint8_t buf_swap = 0;
	uint32_t ADC_data[ADC_PARAMETER_COUNT] = {};
    uint16_t i;                                 /* Iteration counter */

    if (true == transferDone)
    {

		/* Copy data from AFE_DMA fist_half_buffer to FFT lib private memory */
		for(i = 0; i < DEMO_ADC_SAMPLE_COUNT; i++)
		{
			int32_t temp32 = 0; /* Temporary variable  */

			temp32     = g_Adc1ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 0)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_U1_INDEX]  = (frac32) temp32 << 8; /* FFT needs 24 bit data */
			/* Voltage sample phase L2 */
			temp32     = g_Adc1ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 1)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_U2_INDEX]  = (frac32) temp32 << 8; /* FFT needs 24 bit data */
			/* Voltage sample phase L3 */
			temp32     = g_Adc1ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 2)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_U3_INDEX]  = (frac32) temp32 << 8; /* FFT needs 24 bit data */

			/* Current sample phase L1 */
			temp32     = g_Adc0ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 0)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_I1_INDEX] = (frac32) temp32 << 8; /* FFT needs 24 bit data */

			/* Current sample phase L2 */
			temp32     = g_Adc0ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 1)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_I2_INDEX] = (frac32) temp32 << 8; /* FFT needs 24 bit data */

			/* Current sample phase L3 */
			temp32     = g_Adc0ConvResult[buf_swap][((i * ADC_NO_DATA_WORDS) + 2)];
			temp32     = (temp32 & ADC_RESFIFO_D_MASK);
			temp32     = temp32 - 0x8000; /* Deduct DC offset */
			ADC_data[ADC_I3_INDEX] = (frac32) temp32 << 8; /* FFT needs 24 bit data */

			uint32_t current_sample = Metering_SampleAdd(kMetADCMode, ADC_data[ADC_U1_INDEX], ADC_data[ADC_U2_INDEX], ADC_data[ADC_U3_INDEX],
					ADC_data[ADC_I1_INDEX], ADC_data[ADC_I2_INDEX], ADC_data[ADC_I3_INDEX]);
		}
	   /* Change data buffer "pointer" for next cycle */
		buf_swap ^=  1;

		Metering_SampleFinish(kMetADCMode);
		/* Invoke metering process */
		NVIC_CallIsr(SAR_PROCESS_IRQ);

	}/* End of buffer feeding*/
}

/**
 * @brief Configure the DMA for ADC0 
 * 
 */

static void DMA_ADC0_Configure(void)
{
    dma_channel_config_t dmaChannelConfigStruct_21;

    g_XferConfig_21 = DMA_CHANNEL_XFER(true,                       /* Reload link descriptor after current exhaust, */
                                    false,                         /* Software trigger once and let it run forever. */
                                    true,                          /* Enable interruptA. */
                                    false,                         /* Not enable interruptB. */
                                    sizeof(uint32_t),              /* Dma transfer width. */
                                    kDMA_AddressInterleave0xWidth, /* Dma source address no interleave  */
                                    kDMA_AddressInterleave1xWidth, /* Dma destination address no interleave  */
                                    DEMO_CADC_SAMPLE_COUNT * sizeof(uint32_t) /* Dma transfer byte. */
    );

    DMA_EnableChannel(DEMO_DMA_BASE, DEMO_DMA_ADC0_CHANNEL);

    /* Configure DMA. */
    DMA_CreateHandle(&g_DmaHandleStruct_21, DEMO_DMA_BASE, DEMO_DMA_ADC0_CHANNEL);
    DMA_SetCallback(&g_DmaHandleStruct_21, DEMO_DMA_Callback, NULL);

    /* Prepare and submit the transfer. */
    DMA_PrepareChannelTransfer(&dmaChannelConfigStruct_21,                              /* DMA channel transfer configuration structure. */
                               (void *)DEMO_LPADC0_RESFIFO_REG_ADDR,                    /* DMA transfer source address. */
                               (void *)g_Adc0ConvResult,                                /* DMA transfer destination address. */
                               g_XferConfig_21,                                         /* Xfer configuration */
                               DEMO_DMA_TRANSFER_TYPE,                                  /* DMA transfer type. */
							   NULL,													/* Software trigger */
                               (dma_descriptor_t *)&(s_dma_table[0])                    /* Address of next descriptor. */
    );
    DMA_SubmitChannelTransfer(&g_DmaHandleStruct_21, &dmaChannelConfigStruct_21);

    DMA_SetupDescriptor((dma_descriptor_t *)&(s_dma_table[0]), g_XferConfig_21, (void *)DEMO_LPADC0_RESFIFO_REG_ADDR,
                        (void *)g_Adc0ConvResult[0], (dma_descriptor_t *)&(s_dma_table[4]));
    DMA_SetupDescriptor((dma_descriptor_t *)&(s_dma_table[4]), g_XferConfig_21, (void *)DEMO_LPADC0_RESFIFO_REG_ADDR,
                        (void *)g_Adc0ConvResult[1], (dma_descriptor_t *)&(s_dma_table[0]));

    /* Set the DMA trigger */
    DMA_StartTransfer(&g_DmaHandleStruct_21);

}

/**
 * @brief Configure the DMA for ADC1 
 * 
 */
static void DMA_ADC1_Configure(void)
{
    dma_channel_config_t dmaChannelConfigStruct_27;

    g_XferConfig_27 = DMA_CHANNEL_XFER(true,                       						/* Reload link descriptor after current exhaust, */
									false,                         						/* Software trigger once and let it run forever. */
                                    true,                          						/* Enable interruptA. */
                                    false,                         						/* Not enable interruptB. */
                                    sizeof(uint32_t),              						/* Dma transfer width. */
                                    kDMA_AddressInterleave0xWidth, 						/* Dma source address no interleave  */
                                    kDMA_AddressInterleave1xWidth, 						/* Dma destination address no interleave  */
									DEMO_CADC_SAMPLE_COUNT * sizeof(uint32_t) 			/* Dma transfer byte. */
    );

    DMA_EnableChannel(DEMO_DMA_BASE, DEMO_DMA_ADC1_CHANNEL);

    /* Configure DMA. */
    DMA_CreateHandle(&g_DmaHandleStruct_27, DEMO_DMA_BASE, DEMO_DMA_ADC1_CHANNEL);

    /* Prepare and submit the transfer. */
    DMA_PrepareChannelTransfer(&dmaChannelConfigStruct_27,                              /* DMA channel transfer configuration structure. */
                               (void *)DEMO_LPADC1_RESFIFO_REG_ADDR,                    /* DMA transfer source address. */
                               (void *)g_Adc1ConvResult,                                /* DMA transfer destination address. */
                               g_XferConfig_27,                                         /* Xfer configuration */
                               DEMO_DMA_TRANSFER_TYPE,                                  /* DMA transfer type. */
							   NULL,													/* Software trigger */
                               (dma_descriptor_t *)&(s_dma_table[8])                    /* Address of next descriptor. */
    );
    DMA_SubmitChannelTransfer(&g_DmaHandleStruct_27, &dmaChannelConfigStruct_27);

    DMA_SetupDescriptor((dma_descriptor_t *)&(s_dma_table[8]), g_XferConfig_27, (void *)DEMO_LPADC1_RESFIFO_REG_ADDR,
                        (void *)g_Adc1ConvResult[0], (dma_descriptor_t *)&(s_dma_table[12]));
    DMA_SetupDescriptor((dma_descriptor_t *)&(s_dma_table[12]), g_XferConfig_27, (void *)DEMO_LPADC1_RESFIFO_REG_ADDR,
                        (void *)g_Adc1ConvResult[1], (dma_descriptor_t *)&(s_dma_table[8]));

    /* Set the DMA trigger */
    DMA_StartTransfer(&g_DmaHandleStruct_27);
}

/*!
 * @brief ADC initialization function
 */
void ADC_CP_PP_Init(void)
{
	ClockInit();
	ADC0_Init();
	ADC0_CP_PP_Configure();
    CTIMER0_Init();

}

void ADC_Metrology_Init(void)
{
	INPUTMUX_Init(INPUTMUX);

    /* Disable VREF power down */
    POWER_DisablePD(kPDRUNCFG_PD_VREF);
    POWER_DisablePD(kPDRUNCFG_PD_OPAMP0);
    POWER_DisablePD(kPDRUNCFG_PD_OPAMP1);
    POWER_DisablePD(kPDRUNCFG_PD_OPAMP2);

    VREF_Configuration();  /* Initialize the VREF */
    OPAMP_Configuration(); /* Initialize the OPAMP */
    VREF_SetTrim21Val(VREF, 7); /* Trim VREFOUT to ~1.65V */
	ADC0_Current_Configure();
	ADC1_Init();
	ADC1_Voltage_Configure();

    DMA_Init(DEMO_DMA_BASE);

	DMA_ADC0_Configure();
	DMA_ADC1_Configure();
	/* Start the trigger only when we want Metrology with internal ADC */
    CTIMER4_Init();

}

void ADC_Metrology_Deinit(void)
{
	/* Just stop the clock and the whole flow is stopped as no DMA trigger */
	CTIMER4_Deinit();
}

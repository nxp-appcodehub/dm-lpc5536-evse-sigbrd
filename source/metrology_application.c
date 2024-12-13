/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "fsl_debug_console.h"
#include "fraclib.h"           /* fractional library header */
#include "meterlibfft.h"       /* metering library header */
#include "inputdata.h"      /* library of a typical harmonic signals */

#include "metrology_application.h"
#include "external_afe.h"
#include "adc_manager.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define INP_SAMPLES MAX(FFT_SAMPLES, MAX(ADC_INP_SAMPLES, AFE_INP_SAMPLES))
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool met_mode_init = false;

/*******************************************************************************
 * Buffers definitions                                                         *
 ******************************************************************************/
/* multiplexed mandatory buffers (time domain / frequency domain in the Cartesian form) */
/* U-ADC output buffer/FFT real part output buffer (Ph1) */
frac24 u1_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];
/* I-ADC output buffer/FFT real part output buffer (Ph1) */
frac24 i1_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];
/* U-ADC output buffer/FFT real part output buffer (Ph2) */
frac24 u2_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];
/* I-ADC output buffer/FFT real part output buffer (Ph2) */
frac24 i2_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];
/* U-ADC output buffer/FFT real part output buffer (Ph3) */
frac24 u3_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];
/* I-ADC output buffer/FFT real part output buffer (Ph3) */
frac24 i3_re[kMetModeSize][NUM_MET_BUFFERS][INP_SAMPLES];

/* dedicated mandatory buffers (frequency domain in the Cartesian form) */
frac24 u1_im[kMetModeSize][FFT_SAMPLES];     /* U-FFT imaginary part output buffer (Ph1) */
frac24 i1_im[kMetModeSize][FFT_SAMPLES];     /* I-FFT imaginary part output buffer (Ph1) */
frac24 u2_im[kMetModeSize][FFT_SAMPLES];     /* U-FFT imaginary part output buffer (Ph2) */
frac24 i2_im[kMetModeSize][FFT_SAMPLES];     /* I-FFT imaginary part output buffer (Ph2) */
frac24 u3_im[kMetModeSize][FFT_SAMPLES];     /* U-FFT imaginary part output buffer (Ph3) */
frac24 i3_im[kMetModeSize][FFT_SAMPLES];     /* I-FFT imaginary part output buffer (Ph3) */

/* auxiliary buffers (frequency domain in the Polar form) */
frac24 i1_mag[AUX_SAMPLES];    /* U-magnitudes output buffer (Ph1) */
frac24 u1_mag[AUX_SAMPLES];    /* I-magnitudes output buffer (Ph1) */
frac24 i2_mag[AUX_SAMPLES];    /* U-magnitudes output buffer (Ph2) */
frac24 u2_mag[AUX_SAMPLES];    /* I-magnitudes output buffer (Ph2) */
frac24 i3_mag[AUX_SAMPLES];    /* U-magnitudes output buffer (Ph3) */
frac24 u3_mag[AUX_SAMPLES];    /* I-magnitudes output buffer (Ph3) */
long i1_ph[AUX_SAMPLES];       /* U-phases output buffer (Ph1) */
long u1_ph[AUX_SAMPLES];       /* I-phases output buffer (Ph1) */
long i2_ph[AUX_SAMPLES];       /* U-phases output buffer (Ph2) */
long u2_ph[AUX_SAMPLES];       /* I-phases output buffer (Ph2) */
long i3_ph[AUX_SAMPLES];       /* U-phases output buffer (Ph3) */
long u3_ph[AUX_SAMPLES];       /* I-phases output buffer (Ph3) */

/* auxiliary U-I phase shift buffers [0.001°] - used for S/W phase shift correction (Ph1..Ph3) */
long shift1[kMetModeSize][FFT_SAMPLES/2] = {{0,-5300,0,0,0,-4500,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		                         {0,-5300,0,0,0,-4500,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
long shift2[kMetModeSize][FFT_SAMPLES/2] = {0};
long shift3[kMetModeSize][FFT_SAMPLES/2] = {{0,-90000,0,0,0,-90000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		                         {0,-90000,0,0,0,-90000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

unsigned long frequency = DEMO_DISTRIBUTION_FREQUENCY * 1000;       /* line frequency [mHz] - should be measured finaly */
unsigned long current_sample[kMetModeSize];
unsigned long adc_samples[kMetModeSize];             /* number of true ADC samples (it can be slightly changed
                                          due to mains frequency varying in real application) */
long fcn_out;                          /* metering functions output or functions error state */
tMETERLIBFFT3PH_DATA ui[kMetModeSize];               /* 3-PH main metering structure */
double urms1[kMetModeSize],irms1[kMetModeSize],p1[kMetModeSize],q1[kMetModeSize],s1[kMetModeSize],pf1[kMetModeSize],thdu1[kMetModeSize],thdi1[kMetModeSize];/* non-billing (instantaneous) metering values Ph1*/
double urms2[kMetModeSize],irms2[kMetModeSize],p2[kMetModeSize],q2[kMetModeSize],s2[kMetModeSize],pf2[kMetModeSize],thdu2[kMetModeSize],thdi2[kMetModeSize];/* non-billing (instantaneous) metering values Ph2*/
double urms3[kMetModeSize],irms3[kMetModeSize],p3[kMetModeSize],q3[kMetModeSize],s3[kMetModeSize],pf3[kMetModeSize],thdu3[kMetModeSize],thdi3[kMetModeSize];/* non-billing (instantaneous) metering values Ph3*/
double u12_ph[kMetModeSize], u13_ph[kMetModeSize], u23_ph[kMetModeSize];        /* phase shifts */
unsigned long whi[kMetModeSize],whe[kMetModeSize],varhi[kMetModeSize],varhe[kMetModeSize];    /* billing (accumulative) metering values */
int wr_buffer[kMetModeSize] = {0, 0};
int rd_buffer[kMetModeSize] = {1, 1};

long read_period = 0;                 /* reading period for non-billing averaged values */
unsigned long tmr_cmp;                /* timer compare register value for energy LED driving */
long seq;                             /* sense of rotation */
frac24 afe1_offs = 0;                 /* AFE1 I-offset correction value [bit] */
frac24 afe2_offs = 0;                 /* AFE2 I-offset correction value [bit] */
frac24 afe3_offs = 0;                 /* AFE3 I-offset correction value [bit] */

frac24 adc1_offs = 0;            /* AFE1 I-offset correction value [bit] */
frac24 adc2_offs = 0;                 /* AFE2 I-offset correction value [bit] */
frac24 adc3_offs = 0;                 /* AFE3 I-offset correction value [bit] */

/* The scope of met_mode variable should remain within metrology_application.
 * Use exposed APIs - Metering_GetMeterologyMode and Metering_SetMeterologyMode
 * instead of extern-ing this variable outside metrology_application.
 */
met_mode_t met_mode = kMetModeSize;

/*******************************************************************************
 * Code
 ******************************************************************************/

met_mode_t Metering_GetMetrologyMode(void)
{
	return met_mode;
}

void Metering_ConfigureMetrologyMode(met_mode_t metrology_mode)
{
	if (metrology_mode != met_mode)
	{
		Metering_MetrologyHWDeinit(met_mode);
		met_mode = metrology_mode;
		Metering_MetrologyHWInit(met_mode);
	}
}

void Metering_SetMetrologyMode(met_mode_t metrology_mode)
{
	met_mode = metrology_mode;
}

void Metering_MetrologyHWDeinit(met_mode_t metrology_mode)
{

	switch(metrology_mode)
	{
		case kMetAFEMode:
			AFE_DeInit();
			Metering_SampleReset(metrology_mode);
			break;
		case kMetADCMode:
			ADC_Metrology_Deinit();
			Metering_SampleReset(metrology_mode);
			break;
		default :
			break;
	}
	met_mode_init =  false;
}

void Metering_MetrologyHWInit(met_mode_t metrology_mode)
{
	switch(metrology_mode)
	{
		case kMetAFEMode:
			/* Save number of samples added */
			adc_samples[kMetAFEMode] = 0;
			current_sample[kMetAFEMode] = 0;
			rd_buffer[kMetAFEMode] = 0;
			/* switch buffer at the completion of one sin~wave */
			wr_buffer[kMetAFEMode] = 0;

			AFE_Init();
			break;
		case kMetADCMode:
			/* Save number of samples added */
			adc_samples[kMetADCMode] = 0;
			current_sample[kMetADCMode] = 0;
			rd_buffer[kMetADCMode] = 0;
			/* switch buffer at the completion of one sin~wave */
			wr_buffer[kMetADCMode] = 0;

			ADC_Metrology_Init();
			break;
		default :
			break;
	}
	met_mode_init = true;
}

/*!
 * @brief Meterology Initialization
 */
void Metering_Init(met_mode_t met_mode)
{
	PRINTF("Initializing metering\r\n");

	if(met_mode == kMetAFEMode)
	{
		/* Mandatory initialization section - for main FFT calculation             */
		fcn_out = METERLIBFFT3PH_InitParam(&ui[met_mode],FFT_SAMPLES, SENS_PROP, IMP2000, IMP2000, EN_RES10);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh1(&ui[met_mode], 634, 64.00, &afe1_offs, 0.0, 0.0);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh2(&ui[met_mode], 634, 64.00, &afe2_offs, 0.0, 0.0);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh3(&ui[met_mode], 634, 64.00, &afe3_offs, 0.0, 0.0);

		/* Energy registers can be zeroed or filled-up with some initial values    */
		METERLIBFFT_SetEnergy(ui[met_mode], 0, 0, 0, 0);

		/* Auxiliary initialization - for magnitudes and phase shifts computing only */
		METERLIBFFT3PH_InitAuxBuffPh1(&ui[met_mode], u1_mag, i1_mag, u1_ph, i1_ph);
		METERLIBFFT3PH_InitAuxBuffPh2(&ui[met_mode], u2_mag, i2_mag, u2_ph, i2_ph);
		METERLIBFFT3PH_InitAuxBuffPh3(&ui[met_mode], u3_mag, i3_mag, u3_ph, i3_ph);

		wr_buffer[met_mode] = 0;
		rd_buffer[met_mode] = 1;
	}
	else if(met_mode == kMetADCMode)
	{
		/* Mandatory initialization section - for main FFT calculation             */
		fcn_out = METERLIBFFT3PH_InitParam(&ui[met_mode],FFT_SAMPLES, SENS_PROP, IMP2000, IMP2000, EN_RES10);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh1(&ui[met_mode], 506, 65.50, &adc1_offs, 0.0, 0.0);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh2(&ui[met_mode], 506, 65.50, &adc2_offs, 0.0, 0.0);
		fcn_out = METERLIBFFT3PH_SetCalibCoeffPh3(&ui[met_mode], 506, 65.50, &adc3_offs, 0.0, 0.0);

		/* Energy registers can be zeroed or filled-up with some initial values    */
		METERLIBFFT_SetEnergy(ui[met_mode], 0, 0, 0, 0);

		wr_buffer[met_mode] = 0;
		rd_buffer[met_mode] = 1;
	}
}

void Metering_SampleReset(met_mode_t met_mode)
{
	current_sample[met_mode] = 0;
}

uint32_t Metering_SampleAdd(met_mode_t met_mode, frac24 u1, frac24 u2, frac24 u3, frac24 i1, frac24 i2, frac24 i3)
{
	if (met_mode == kMetAFEMode || met_mode == kMetADCMode)
	{
		if (current_sample[met_mode] < INP_SAMPLES)
		{
			u1_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = u1;
			u2_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = u2;
			u3_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = u3;
			i1_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = i1;
			i2_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = i2;
			i3_re[met_mode][wr_buffer[met_mode]][current_sample[met_mode]] = i3;
			current_sample[met_mode]++;
		}
		return current_sample[met_mode];
	}

	return 0;
}

void Metering_SampleFinish(met_mode_t met_mode)
{
	if (met_mode == kMetAFEMode || met_mode == kMetADCMode)
	{
		/* Save number of samples added */
		adc_samples[met_mode] = current_sample[met_mode];
		/* Reset the buffer */
		current_sample[met_mode] = 0;
		/* Save written buffer */
		rd_buffer[met_mode] = wr_buffer[met_mode];
		/* switch buffer at the completion of one sin~wave */
		wr_buffer[met_mode] = 1 - wr_buffer[met_mode];
	}
}

/*!
 * @brief Meterology Processing
 */
void Metering_Process(void *metering_mode)
{

	met_mode_t met_mode = (met_mode_t) metering_mode;

	METERLIBFFT3PH_InitMainBuffPh1(&ui[met_mode], u1_re[met_mode][rd_buffer[met_mode]], i1_re[met_mode][rd_buffer[met_mode]], u1_im[met_mode], i1_im[met_mode], shift1[met_mode]);
	METERLIBFFT3PH_InitMainBuffPh2(&ui[met_mode], u2_re[met_mode][rd_buffer[met_mode]], i2_re[met_mode][rd_buffer[met_mode]], u2_im[met_mode], i2_im[met_mode], shift2[met_mode]);
	METERLIBFFT3PH_InitMainBuffPh3(&ui[met_mode], u3_re[met_mode][rd_buffer[met_mode]], i3_re[met_mode][rd_buffer[met_mode]], u3_im[met_mode], i3_im[met_mode], shift3[met_mode]);

	if (adc_samples[met_mode] > FFT_SAMPLES)
	{
		/* performs interpolation only for asynchronous processing */
		fcn_out = METERLIBFFT3PH_Interpolation(&ui[met_mode], ORD2, ORD3, adc_samples[met_mode]);
	}

	/* main calculation (FFT, scaling, averaging) */
	METERLIBFFT3PH_CalcMain(&ui[met_mode]);

	if (met_mode == kMetAFEMode)
	{
		/* gets max. 32 FFT magnitudes for all lines (auxiliary output only) */
		METERLIBFFT3PH_GetMagnitudesPh1(&ui[met_mode], AUX_SAMPLES);
		METERLIBFFT3PH_GetMagnitudesPh2(&ui[met_mode], AUX_SAMPLES);
		METERLIBFFT3PH_GetMagnitudesPh3(&ui[met_mode], AUX_SAMPLES);

		/* gets max. 32 FFT phases for all lines (auxiliary output only) */
		METERLIBFFT3PH_GetPhasesPh1(&ui[met_mode], AUX_SAMPLES);
		METERLIBFFT3PH_GetPhasesPh2(&ui[met_mode], AUX_SAMPLES);
		METERLIBFFT3PH_GetPhasesPh3(&ui[met_mode], AUX_SAMPLES);
	}

	/* sense of rotation measurement */
	seq = METERLIBFFT3PH_GetRotation(&ui[met_mode], &u12_ph[met_mode], &u13_ph[met_mode], &u23_ph[met_mode]);

	/* calculates active energy increment and threshold value for active energy timer */
	//tmr_cmp = TMRCMPVAL(METERLIBFFT3PH_CalcWattHours(&ui[met_mode], &whi, &whe, frequency), TMRCLK);
	METERLIBFFT3PH_CalcWattHours(&ui[met_mode], &whi[met_mode], &whe[met_mode], frequency);
	/* calculates reactive energy increment and threshold value for reactive energy timer */
	//tmr_cmp = TMRCMPVAL(METERLIBFFT3PH_CalcVarHours(&ui[met_mode], &varhi, &varhe, frequency), TMRCLK);
	METERLIBFFT3PH_CalcVarHours(&ui[met_mode], &varhi[met_mode], &varhe[met_mode], frequency);

	/* reads all non-billing instantaneous metering values (all lines) */
	METERLIBFFT3PH_GetInstValuesPh1(&ui[met_mode], &urms1[met_mode], &irms1[met_mode], &p1[met_mode], &q1[met_mode], &s1[met_mode], &pf1[met_mode], &thdu1[met_mode], &thdi1[met_mode]);
	METERLIBFFT3PH_GetInstValuesPh2(&ui[met_mode], &urms2[met_mode], &irms2[met_mode], &p2[met_mode], &q2[met_mode], &s2[met_mode], &pf2[met_mode], &thdu2[met_mode], &thdi2[met_mode]);
	METERLIBFFT3PH_GetInstValuesPh3(&ui[met_mode], &urms3[met_mode], &irms3[met_mode], &p3[met_mode], &q3[met_mode], &s3[met_mode], &pf3[met_mode], &thdu3[met_mode], &thdi3[met_mode]);

	/* reads all non-billing averaged metering values every 50-th cycle */
	if (++read_period == 50)
	{ /* simulate asynchronous reading of all averaged values */
		METERLIBFFT3PH_GetAvrgValuesPh1(&ui[met_mode], &urms1[met_mode], &irms1[met_mode], &p1[met_mode], &q1[met_mode], &s1[met_mode], &pf1[met_mode], &thdu1[met_mode], &thdi1[met_mode]);
		METERLIBFFT3PH_GetAvrgValuesPh2(&ui[met_mode], &urms2[met_mode], &irms2[met_mode], &p2[met_mode], &q2[met_mode], &s2[met_mode], &pf2[met_mode], &thdu2[met_mode], &thdi2[met_mode]);
		METERLIBFFT3PH_GetAvrgValuesPh3(&ui[met_mode], &urms3[met_mode], &irms3[met_mode], &p3[met_mode], &q3[met_mode], &s3[met_mode], &pf3[met_mode], &thdu3[met_mode], &thdi3[met_mode]);
		read_period = 0;
	}
}

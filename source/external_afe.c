/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_spi.h"
#include "fsl_spi_dma.h"
#include "fsl_dma.h"
#include "fsl_pint.h"
#include "fsl_inputmux.h"
#include <stdbool.h>
#include "swisr.h"
#include "fsl_crc.h"
#include "external_afe.h"
#include "metrology_application.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SPI_MASTER                SPI7
#define SPI_MASTER_IRQ            FLEXCOMM7_IRQn
#define SPI_MASTER_CLK_SRC        kCLOCK_Flexcomm7
#define SPI_MASTER_CLK_FREQ       CLOCK_GetFlexCommClkFreq(7U)
#define SPI_MASTER_SSEL           1
#define DMA                       DMA0
#define SPI_MASTER_RX_CHANNEL     18
#define SPI_MASTER_TX_CHANNEL     19
#define MASTER_SPI_SPOL           kSPI_SpolActiveAllLow
#define TRANSFER_SIZE             12
#define COMMAND_SIZE              2
#define CRC_SIZE                  2

#define DEMO_PINT_PIN_INT2_SRC    kINPUTMUX_GpioPort0Pin19ToPintsel
#define AFE_SPI_PKT_RCV_CNT       5U

DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Rx_data_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Rx_data_crc_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Rx_command_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Rx_command_crc_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Tx_data_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Tx_data_crc_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Tx_command_descriptor, 2);
DMA_ALLOCATE_LINK_DESCRIPTORS(spi_Tx_command_crc_descriptor, 2);

enum _afe_params_t
{
	AFE_U1_INDEX,
	AFE_U2_INDEX,
	AFE_U3_INDEX,
	AFE_I1_INDEX,
	AFE_I2_INDEX,
	AFE_I3_INDEX,
	AFE_PARAMETER_COUNT
};

enum _spi_dma_states_t
{
	kSPI_Idle = 0x0, /*!< SPI is idle state */
	kSPI_Busy        /*!< SPI is busy transferring data. */
};

/*! @brief CRC type of transpose of read write data */
typedef enum _crc_transpose_type
{
    kCrcTransposeNone         = 0U, /*! No transpose  */
    kCrcTransposeBits         = 1U, /*! Tranpose bits in bytes  */
    kCrcTransposeBitsAndBytes = 2U, /*! Transpose bytes and bits in bytes */
    kCrcTransposeBytes        = 3U, /*! Transpose bytes */
} afe_crc_transpose_type_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void AFE_SPIRxDMACallbackCustomCallback(dma_handle_t *handle, void *userData, bool transferDone, uint32_t intmode);
static void AFE_MasterInit(void);
static void AFE_MasterDMASetup(void);
static void AFE_MasterStartDMATransfer(void);
static void AFE_InitCrc16CcittFalse(CRC_Type *base, uint32_t seed);
static uint32_t AFE_GetCRC(uint8_t *data, uint8_t datasize);
static void AFE_DeInterleaveDMABuffer(void);
static void AFE_SPIDMAInit(void);
static status_t AFE_SPIMasterTransferDMACustom(SPI_Type *base, spi_dma_handle_t *handle, spi_transfer_t *xfer);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t spiTxData[32] = {0};
volatile bool isTransferCompleted = true;
dma_handle_t masterTxHandle;
dma_handle_t masterRxHandle;
spi_dma_handle_t masterHandle;

uint16_t masterRxCommand[2] = {0};
uint16_t masterRxCRC[2] = {0};
uint16_t masterRxData[TRANSFER_SIZE] = {0};
uint8_t masterTxData[TRANSFER_SIZE] = {0}; /* Just a placeholder */

CRC_Type *base = CRC0;

int SPI_callback_count = 0;

uint32_t CRC_err_count = 0;
bool CRC_init_done = false;

bool AFE_SPI_Detected = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief This function is a callback for DMA-Rx channel. It will call the AFE_DeInterleaveDMABuffer()
 *
 */
static void AFE_SPIRxDMACallbackCustomCallback(dma_handle_t *handle, void *userData, bool transferDone, uint32_t intmode)
{
	if(transferDone)
	{
		SPI_callback_count++;
		isTransferCompleted = true;
		AFE_DeInterleaveDMABuffer();
	}
}
/*!
 * @brief This function will set up SPI module.
 *
 */
static void AFE_MasterInit(void)
{
	/* SPI init */
	uint32_t srcClock_Hz = 0U;
	spi_master_config_t masterConfig;
	srcClock_Hz = SPI_MASTER_CLK_FREQ;

	SPI_MasterGetDefaultConfig(&masterConfig);
	masterConfig.sselNum = (spi_ssel_t)SPI_MASTER_SSEL;
	masterConfig.sselPol = (spi_spol_t)MASTER_SPI_SPOL;
	masterConfig.baudRate_Bps = 4000000;        /* SPI data rate */
	masterConfig.delayConfig.transferDelay = 5; /* SPI clock cycles */
	masterConfig.delayConfig.postDelay = 5;     /* SPI clock cycles */
	masterConfig.dataWidth = kSPI_Data16Bits;
	SPI_MasterInit(SPI_MASTER, &masterConfig, srcClock_Hz);
}
/*!
 * @brief This function will set up DMA module.
 *
 */
static void AFE_MasterDMASetup(void)
{
	/* DMA init has already been done in SAR ADC */

	/* Configure the DMA channel,priority and handle. */
	DMA_EnableChannel(DMA, SPI_MASTER_TX_CHANNEL);
	DMA_EnableChannel(DMA, SPI_MASTER_RX_CHANNEL);
	DMA_SetChannelPriority(DMA, SPI_MASTER_TX_CHANNEL, kDMA_ChannelPriority3);
	DMA_SetChannelPriority(DMA, SPI_MASTER_RX_CHANNEL, kDMA_ChannelPriority2);

	DMA_CreateHandle(&masterTxHandle, DMA, SPI_MASTER_TX_CHANNEL);
	DMA_CreateHandle(&masterRxHandle, DMA, SPI_MASTER_RX_CHANNEL);

	/* enabling hardware trigger */
	dma_channel_trigger_t trigger;
	trigger.type  = kDMA_RisingEdgeTrigger;
	trigger.burst = kDMA_SingleTransfer;
	trigger.wrap  = kDMA_NoWrap;
	DMA_ConfigureChannelTrigger(DMA,  SPI_MASTER_TX_CHANNEL, &trigger);
}
/*!
 * @brief This function will create DMA handles and set callback for DMA Rx.
 *
 */
static void AFE_MasterStartDMATransfer(void)
{
	spi_transfer_t masterXfer;

	/* Set up handle for spi master */
	SPI_MasterTransferCreateHandleDMA(SPI_MASTER, &masterHandle, NULL, NULL, &masterTxHandle,
			&masterRxHandle);

	DMA_SetCallback(&masterRxHandle, AFE_SPIRxDMACallbackCustomCallback, NULL);

	/* Start master transfer */
	masterXfer.dataSize    = sizeof(masterTxData[0])*TRANSFER_SIZE ;
	masterXfer.configFlags = kSPI_FrameAssert;

	masterXfer.txData      = (uint8_t *)&(masterTxData[0]);
	masterXfer.rxData      = (uint8_t *)&(masterRxData[0]);

	AFE_SPIMasterTransferDMACustom(SPI_MASTER, &masterHandle, &masterXfer);
}

static uint32_t AFE_GetCRC(uint8_t *data, uint8_t datasize)
{
	/* ***************
	 * CRC-16/CCITT-FALSE *
	 *************** */
	uint32_t checksum16 = 0;

	crc_config_t config;
	CRC_GetDefaultConfig(&config);

	/* pre-compute value for CRC control registger based on user configuraton without WAS field */
	int crcControl = 0U | CRC_CTRL_TOT((config.reflectIn) ? kCrcTransposeBitsAndBytes : kCrcTransposeBytes)
					| CRC_CTRL_TOTR((config.reflectOut) ? kCrcTransposeBitsAndBytes : kCrcTransposeNone) |
					 CRC_CTRL_FXOR(config.complementChecksum) | CRC_CTRL_TCRC(config.crcBits);
	/* make sure the control register is clear - WAS is deasserted, and protocol is set */
	base->CTRL = crcControl;
	/* write polynomial register ( for CRC-16/CCITT_FALSE, polynomial = 0x1021 ) */
	base->GPOLY = config.polynomial;
	/* write pre-computed control register value along with WAS to start checksum computation */
	base->CTRL = crcControl | CRC_CTRL_WAS(true);
	/* write seed (initial checksum for CRC-16/CCITT_FALSE is 0xFFFF ) */
	base->DATA = config.seed;
	/* deassert WAS by writing pre-computed CRC control register value */
	base->CTRL = crcControl;

	CRC_WriteData(base, data, datasize);

	checksum16 = CRC_Get16bitResult(base);

	return checksum16;
}


/*!
 * @brief This function will help to separate out the mixed samples to
 * individual channel specific buffers.
 *
 */
static void AFE_DeInterleaveDMABuffer(void)
{
	bool err = false;
	uint32_t AFE_data[AFE_PARAMETER_COUNT] = {};
	uint32_t crc_val_calculated;
	uint32_t crc_val_received;
	static uint8_t detect_count = AFE_SPI_PKT_RCV_CNT;

	/* 24 bytes = 12 packets of 2 bytes = 16 bit Higher Nibble then Lower nibble */

	/* store samples*/

	AFE_data[AFE_U1_INDEX] = (masterRxData[0]<<16)|(masterRxData[1]); /* U1 */
	AFE_data[AFE_U2_INDEX] = (masterRxData[2]<<16)|(masterRxData[3]); /* U2 */
	AFE_data[AFE_U3_INDEX] = (masterRxData[4]<<16)|(masterRxData[5]); /* U3 */
	AFE_data[AFE_I1_INDEX] = (masterRxData[6]<<16)|(masterRxData[7]); /* I1 */
	AFE_data[AFE_I2_INDEX] = (masterRxData[8]<<16)|(masterRxData[9]); /* I2 */
	AFE_data[AFE_I3_INDEX] = (masterRxData[10]<<16)|(masterRxData[11]); /* I3 */

	crc_val_calculated = AFE_GetCRC((uint8_t *)AFE_data, sizeof(AFE_data));
	crc_val_received = masterRxCRC[0]<<16 | masterRxCRC[1];

	if(crc_val_calculated != crc_val_received)
	{
		CRC_err_count++;
		err = true;

		/* In case of CRC error in a packet, clear the buffer to drop the particular cycle */
		Metering_SampleReset(kMetAFEMode);
	}
	else
	{
		uint32_t current_sample = Metering_SampleAdd(kMetAFEMode, AFE_data[AFE_U1_INDEX], AFE_data[AFE_U2_INDEX], AFE_data[AFE_U3_INDEX],
				AFE_data[AFE_I1_INDEX], AFE_data[AFE_I2_INDEX], AFE_data[AFE_I3_INDEX]);

		if(current_sample == AFE_INP_SAMPLES)
		{
			Metering_SampleFinish(kMetAFEMode);
			/* Invoke metering process */
			NVIC_CallIsr(AFE_PROCESS_IRQ);
		}

		/* Check AFE_SPI_PKT_RCV_CNT  times with no CRC error before confirming SPI detection */
		if (detect_count)
		{
			if(detect_count > 1U)
			{
				detect_count--;
			}
			else
			{
				detect_count--;
				AFE_SPI_Detected = true;
			}
		}
		err = false;
	}
}

/*!
 * @brief This function will provide status of SPI detection.
 *
 */
bool AFE_SPIDetected(void)
{
	return AFE_SPI_Detected;
}
void AFE_SetSPIDetection(bool detection)
{
	AFE_SPI_Detected = detection;
}

/*!
 * @brief This function will initialize DMA and SPI peripherals.
 *
 */
static void AFE_SPIDMAInit()
{
	/* Initialize SPI master with configuration. */
	AFE_MasterInit();
	/* Set up DMA for SPI master TX and RX channel. */
	AFE_MasterDMASetup();

	AFE_MasterStartDMATransfer();
}
/*!
 * @brief Initialize SPI and DMA channels for external AFE interface.
 *
 * This function can be used to initialize the SPI master and DMA channels
 * to drive and get data from external AFE SPI slave.
 */
void AFE_Init()
{
	/* Connect PINT2 to SPI master DMA channel request */
	/* Connect trigger sources to PINT */
	INPUTMUX_Init(INPUTMUX);

	INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt2, DEMO_PINT_PIN_INT2_SRC);

	PINT_Init(PINT);
	/* Setup Pin Interrupt 2 for falling edge */
	PINT_PinInterruptConfig(PINT, kPINT_PinInt2, kPINT_PinIntEnableRiseEdge, /*pint2_intr_callback*/NULL);

	PINT_EnableCallbackByIndex(PINT, kPINT_PinInt2);

	/* Attach PINT output to DMA request trigger */
	INPUTMUX_AttachSignal(INPUTMUX, SPI_MASTER_TX_CHANNEL, kINPUTMUX_PinInt2ToDma0);

	AFE_SPIDMAInit();

	while (!isTransferCompleted)
	{
	}
}

/*!
 * @brief Initialize SPI and DMA channels for external AFE interface.
 *
 * This function can be used to initialize the SPI master and DMA channels
 * to drive and get data from external AFE SPI slave.
 */
void AFE_DeInit(void)
{
	PINT_DisableCallbackByIndex(PINT, kPINT_PinInt2);
	/* Set PINT PIN_INT2 callback to NULL if set to valid callback in AFE_Init */
	SPI_Deinit(SPI_MASTER);
	/* DMA Deinit not done as DMA is used in SAR ADC */
}


/*!
 * @brief This function will help to configure DMA Rx and Tx channels for data transfer.
 *
 */
static status_t AFE_SPIMasterTransferDMACustom(SPI_Type *base, spi_dma_handle_t *handle, spi_transfer_t *xfer)
{
	assert(!((NULL == handle) || (NULL == xfer)));
	uint32_t xfer_cfg = 0 ;
	spi_config_t *spi_config_p;
	uint32_t address;
	void *nextDesc                   = NULL;
	uint32_t firstTimeSize           = 0;
	spi_config_p                     = (spi_config_t *)SPI_GetConfig(base);

	uint8_t bytesPerFrame = (uint8_t)((spi_config_p->dataWidth > kSPI_Data8Bits) ? (sizeof(uint16_t)) : (sizeof(uint8_t)));
	handle->bytesPerFrame = bytesPerFrame;
	uint8_t lastwordBytes = 0U;

	if ((xfer->configFlags & (uint32_t)kSPI_FrameAssert) != 0U)
	{
		handle->lastwordBytes = bytesPerFrame;
		lastwordBytes         = bytesPerFrame;
	}
	else
	{
		handle->lastwordBytes = 0U;
		lastwordBytes         = 0U;
	}

	if ((NULL == handle) || (NULL == xfer))
	{
		return kStatus_InvalidArgument;
	}

	/* Byte size is zero. */
	if (xfer->dataSize == 0U)
	{
		return kStatus_InvalidArgument;
	}

	/* Check if the device is busy */
	if (handle->state == (uint8_t)kSPI_Busy)
	{
		return kStatus_SPI_Busy;
	}
	else
	{
		/* Clear FIFOs before transfer. */
		base->FIFOCFG |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
		base->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;

		handle->state        = (uint8_t)kSPI_Busy;
		handle->transferSize = xfer->dataSize;

		/* receive */
		SPI_EnableRxDMA(base, true);
		address = (uint32_t)&base->FIFORD;
		if (xfer->rxData != NULL)
		{
			handle->rxEndData = xfer->rxData + xfer->dataSize;

			/* Descriptor configuration for receiving response while sending command */
		    xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												false,                                       /* Clear trigger status. */
												false,                                       /* Not enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint16_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave1xWidth,               /* Dma destination address no interleave  */
												sizeof(uint16_t) * (COMMAND_SIZE)            /* Dma transfer byte. */
			);
			/* Descriptor for receiving response while sending command */
			/* This response is meaningless in SigBrd context,
			 * as it's a response to the command sent to AFE just to notify the AFE to start data transmission */
			DMA_SetupDescriptor(&spi_Rx_command_descriptor[0], xfer_cfg, (uint32_t *)address,
					masterRxCommand, &spi_Rx_data_descriptor[0]);

			/* Descriptor configuration for receiving data */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												false,                                       /* Clear trigger status. */
												false,                                       /* Not enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint16_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave1xWidth,               /* Dma destination address no interleave  */
												sizeof(uint16_t) * (TRANSFER_SIZE)           /* Dma transfer byte. */
			);
			/* Descriptor for receiving data (AFE samples) */
			DMA_SetupDescriptor(&spi_Rx_data_descriptor[0], xfer_cfg, (uint32_t *)address,
					xfer->rxData, &spi_Rx_data_crc_descriptor[0]);

			/* Descriptor configuration for receiving CRC */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												false,                                       /* Clear trigger status. */
												true,                                        /* Enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint16_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave1xWidth,               /* Dma destination address no interleave  */
												sizeof(uint16_t) * (CRC_SIZE)        		 /* Dma transfer byte. */
			);
			/* Descriptor for receiving CRC of Data */
			DMA_SetupDescriptor(&spi_Rx_data_crc_descriptor[0], xfer_cfg, (uint32_t *)address,
					masterRxCRC, &spi_Rx_command_descriptor[0]);

			DMA_EnableChannelPeriphRq(handle->rxHandle->base, handle->rxHandle->channel);

			handle->rxNextData = xfer->rxData + firstTimeSize;
		}
		else
		{
			return kStatus_InvalidArgument;
		}

		DMA_SubmitChannelDescriptor(handle->rxHandle, &spi_Rx_command_descriptor[0]);
		handle->rxInProgress = true;
		DMA_StartTransfer(handle->rxHandle);

		/* transmit */
		SPI_EnableTxDMA(base, true);
		address = (uint32_t)&base->FIFOWR;
		if (xfer->txData != NULL)
		{

			/* sending 0 while getting data and its CRC */
			spiTxData[0] = (0x0) |kSPI_FrameDelay | SPI_FIFOWR_LEN(kSPI_Data16Bits)| kSPI_FrameAssert;
			/* sending dummy command */
			spiTxData[1] =  (0b0101100000000110 | spiTxData[0]);
			/* sending dummy Command's CRC */
			spiTxData[2] =  (0b0101100000000110 | spiTxData[0]);

			/* Descriptor configuration for command */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												false,                                       /* Clear trigger status. */
												false,                                       /* Not enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint32_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave0xWidth,               /* Dma destination address no interleave  */
												sizeof(uint32_t) * (COMMAND_SIZE)            /* Dma transfer byte. */
			);
			/* Descriptor for command  */
			DMA_SetupDescriptor(&spi_Tx_command_descriptor[0], xfer_cfg, &spiTxData[1],
								(uint32_t *)address, &spi_Tx_command_crc_descriptor[0]);

			/* Descriptor configuration for command's CRC */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												false,                                       /* Clear trigger status. */
												false,                                       /* Not enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint32_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave0xWidth,               /* Dma destination address no interleave  */
												sizeof(uint32_t) * (CRC_SIZE)                /* Dma transfer byte. */
			);
			/* Descriptor for command's CRC  */
			DMA_SetupDescriptor(&spi_Tx_command_crc_descriptor[0], xfer_cfg, &spiTxData[2],
											(uint32_t *)address, &spi_Tx_data_descriptor[0]);


			/* Descriptor configuration for transfer of dummy data to receive AFE samples */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                      			/* Reload link descriptor after current exhaust, */
			                                    false,                                      		/* Clear trigger status. */
			                                    false,                                     		  	/* Not enable interruptA. */
			                                    false,                                       		/* Not enable interruptB. */
			                                    sizeof(uint32_t),                            		/* Dma transfer width. */
			                                    kDMA_AddressInterleave0xWidth,               		/* Dma source address no interleave  */
			                                    kDMA_AddressInterleave0xWidth,               		/* Dma destination address no interleave  */
												sizeof(uint32_t) * ((TRANSFER_SIZE - CRC_SIZE))     /* Dma transfer byte. */
												/* data except first 4 byte (1 sample), those will come with command's CRC */
		    );
			/* Descriptor for transfer of dummy data to receive AFE samples */
			DMA_SetupDescriptor(&spi_Tx_data_descriptor[0], xfer_cfg, &spiTxData[0],
					(uint32_t *)address, &spi_Tx_data_crc_descriptor[0]);

			/* Descriptor configuration for data's CRC */
			xfer_cfg = DMA_CHANNEL_XFER(		true,                                        /* Reload link descriptor after current exhaust, */
												true,                                        /* Clear trigger status. */
												false,                                       /* Not enable interruptA. */
												false,                                       /* Not enable interruptB. */
												sizeof(uint32_t),                            /* Dma transfer width. */
												kDMA_AddressInterleave0xWidth,               /* Dma source address no interleave  */
												kDMA_AddressInterleave0xWidth,               /* Dma destination address no interleave  */
												sizeof(uint32_t) * (CRC_SIZE)                /* Dma transfer byte. */
			);
			/* Descriptor for data's CRC  */
			DMA_SetupDescriptor(&spi_Tx_data_crc_descriptor[0], xfer_cfg, &spiTxData[0],
														(uint32_t *)address, &spi_Tx_command_descriptor[0]);



			DMA_EnableChannelPeriphRq(handle->txHandle->base, handle->txHandle->channel);

			DMA_SubmitChannelDescriptor(handle->txHandle, &spi_Tx_command_descriptor[0]);
		}
		else
		{
			return kStatus_InvalidArgument;
		}

		DMA_StartTransfer(handle->txHandle);
	}

	return kStatus_Success;
}


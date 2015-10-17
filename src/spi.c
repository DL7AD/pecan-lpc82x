#include "chip.h"
#include "spi.h"
#include "config.h"
#include "time.h"

static SPI_DELAY_CONFIG_T DelayConfigStruct;
static SPI_DATA_SETUP_T XfSetup;

/**
 * SPI Initializer
 */
void SPI_Init(void)
{
	// Configure pins
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL0_IO, RADIO_CS_PIN);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, RADIO_SCK_PIN);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, RADIO_MISO_PIN);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, RADIO_MOSI_PIN);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	// Configure SPI block
	Chip_SPI_Init(LPC_SPI1);
	Chip_SPI_ConfigureSPI(LPC_SPI1, SPI_MODE_MASTER |	// Enable master/Slave mode
						  SPI_CLOCK_CPHA0_CPOL0 |		// Set Clock polarity to 0
						  SPI_CFG_MSB_FIRST_EN |		// Enable MSB first option
						  SPI_CFG_SPOL_LO);				// Chipselect is active low

	// Configure clocking
	Chip_SPIM_SetClockRate(LPC_SPI1, 100000);

	// Configure timings
	DelayConfigStruct.FrameDelay = 0;
	DelayConfigStruct.PostDelay = 0;
	DelayConfigStruct.PreDelay = 0;
	DelayConfigStruct.TransferDelay = 0;
	Chip_SPI_DelayConfig(LPC_SPI1, &DelayConfigStruct);

	// Enable SPI
	Chip_SPI_Enable(LPC_SPI1);
}

void SPI_Transmit(uint16_t *TxBuf, uint16_t *RxBuf, uint32_t len)
{
	// Set packet configuration
	XfSetup.Length = len;
	XfSetup.pTx = TxBuf;
	XfSetup.RxCnt = 0;
	XfSetup.TxCnt = 0;
	XfSetup.DataSize = 8;
	XfSetup.pRx = RxBuf;

	// Transmit packet
	Chip_SPI_RWFrames_Blocking(LPC_SPI1, &XfSetup);
	delay(10); /* FIXME */
}

void SPI_DeInit(void)
{
	// Disable SPI
	Chip_SPI_DeInit(LPC_SPI1);
}

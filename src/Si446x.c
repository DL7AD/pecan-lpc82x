/**
 * Si4464 driver specialized for APRS transmissions. Modulation concept has been taken
 * from Stefan Biereigel DK3SB.
 * @see http://www.github.com/thasti/utrak
 */

#include "Si446x.h"
#include "config.h"
#include "chip.h"
#include "spi.h"
#include "time.h"

#define RF_GPIO_SET(Select)	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, RADIO_GPIO_PIN, Select)
#define RADIO_SDN_SET(Select)	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, RADIO_SDN_PIN, Select)

/**
 * Initializes Si446x transceiver chip. Adjustes the frequency which is shifted by variable
 * oscillator voltage.
 * @param mv Oscillator voltage in mv
 */
bool Si446x_Init(void) {
	// Initialize SPI
	SPI_Init();

	// Configure GPIO pins
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, RADIO_SDN_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, RADIO_GPIO_PIN);

	// Power up transmitter
	RADIO_SDN_SET(false);								// Radio SDN low (power up transmitter)
	delay(1);											// Wait for transmitter to power up

	// Power up (transmits oscillator type)
	uint8_t x3 = (OSC_FREQ >> 24) & 0x0FF;
	uint8_t x2 = (OSC_FREQ >> 16) & 0x0FF;
	uint8_t x1 = (OSC_FREQ >>  8) & 0x0FF;
	uint8_t x0 = (OSC_FREQ >>  0) & 0x0FF;
	uint16_t init_command[] = {0x02, 0x01, 0x01, x3, x2, x1, x0};
	Si446x_write(init_command, 7);

	// Set transmitter GPIOs
	uint16_t gpio_pin_cfg_command[] = {
		0x13,	// Command type = GPIO settings
		0x44,	// GPIO0        0 - PULL_CTL[1bit] - GPIO_MODE[6bit]
		0x00,	// GPIO1        0 - PULL_CTL[1bit] - GPIO_MODE[6bit]
		0x00,	// GPIO2        0 - PULL_CTL[1bit] - GPIO_MODE[6bit]
		0x00,	// GPIO3        0 - PULL_CTL[1bit] - GPIO_MODE[6bit]
		0x00,	// NIRQ
		0x00,	// SDO
		0x00	// GEN_CONFIG
	};
	Si446x_write(gpio_pin_cfg_command, 8);

	// Set misc configuration
	setModem();

	return true;
}

void Si446x_write(uint16_t* txData, uint32_t len) {
	// Transmit data by SPI
	SPI_Transmit(txData, NULL, len);

	// Reqest ACK by Si446x
	uint16_t rx_answer[] = {0x00,0x00};
	while(rx_answer[0] != 0xFF) {

		// Request ACK by Si446x
		uint16_t rx_ready[] = {0x44,0x00};
		SPI_Transmit(rx_ready, rx_answer, 2);

		if(rx_answer[0] != 0xFF) // Si not finished, wait for it
			delay(1);
	}
}

/**
 * Read register from Si446x. First Register CTS is included.
 */
void Si446x_read(uint16_t* txData, uint32_t txLen, uint16_t* rxData, uint32_t rxLen) {
	// Transmit data by SPI
	SPI_Transmit(txData, NULL, txLen);

	// Reqest ACK by Si446x
	uint16_t rx_answer[] = {0x00};
	while(rx_answer[0] != 0xFF) {

		// Request ACK by Si446x
		uint16_t rx_ready[rxLen];
		rx_ready[0] = 0x44;
		SPI_Transmit(rx_ready, rxData, rxLen);

		if(rxData[0] != 0xFF) // Si not finished, wait for it
			delay(1);
	}
}

void sendFrequencyToSi446x(uint32_t freq) {
	// Set the output divider according to recommended ranges given in Si446x datasheet
	uint32_t band = 0;
	uint32_t outdiv;
	if(freq < 705000000UL) {outdiv = 6;  band = 1;};
	if(freq < 525000000UL) {outdiv = 8;  band = 2;};
	if(freq < 353000000UL) {outdiv = 12; band = 3;};
	if(freq < 239000000UL) {outdiv = 16; band = 4;};
	if(freq < 177000000UL) {outdiv = 24; band = 5;};

	// Set the band parameter
	uint32_t sy_sel = 8;
	uint16_t set_band_property_command[] = {0x11, 0x20, 0x01, 0x51, (band + sy_sel)};
	Si446x_write(set_band_property_command, 5);

	// Set the PLL parameters
	uint32_t f_pfd = 2 * OSC_FREQ / outdiv;
	uint32_t n = ((uint32_t)(freq / f_pfd)) - 1;
	float ratio = (float)freq / (float)f_pfd;
	float rest  = ratio - (float)n;

	uint32_t m = (uint32_t)(rest * 524288UL);
	uint32_t m2 = m >> 16;
	uint32_t m1 = (m - m2 * 0x10000) >> 8;
	uint32_t m0 = (m - m2 * 0x10000 - (m1 << 8));

	// Transmit frequency to chip
	uint16_t set_frequency_property_command[] = {0x11, 0x40, 0x04, 0x00, n, m2, m1, m0};
	Si446x_write(set_frequency_property_command, 8);

	uint32_t x = ((((uint32_t)1 << 19) * outdiv * 1300.0)/(2*OSC_FREQ))*2;
	uint8_t x2 = (x >> 16) & 0xFF;
	uint8_t x1 = (x >>  8) & 0xFF;
	uint8_t x0 = (x >>  0) & 0xFF;
	uint16_t set_deviation[] = {0x11, 0x20, 0x03, 0x0a, x2, x1, x0};
	Si446x_write(set_deviation, 7);
}

void setModem() {

	// Disable preamble
	uint16_t disable_preamble[] = {0x11, 0x10, 0x01, 0x00, 0x00};
	Si446x_write(disable_preamble, 5);

	// Do not transmit sync word
	uint16_t no_sync_word[] = {0x11, 0x11, 0x01, 0x11, (0x01 << 7)};
	Si446x_write(no_sync_word, 5);

	// Setup the NCO modulo and oversampling mode
	uint32_t s = OSC_FREQ / 10;
	uint8_t f3 = (s >> 24) & 0xFF;
	uint8_t f2 = (s >> 16) & 0xFF;
	uint8_t f1 = (s >>  8) & 0xFF;
	uint8_t f0 = (s >>  0) & 0xFF;
	uint16_t setup_oversampling[] = {0x11, 0x20, 0x04, 0x06, f3, f2, f1, f0};
	Si446x_write(setup_oversampling, 8);

	// setup the NCO data rate for APRS
	uint16_t setup_data_rate[] = {0x11, 0x20, 0x03, 0x03, 0x00, 0x11, 0x30};
	Si446x_write(setup_data_rate, 7);

	// use 2GFSK from async GPIO0
	uint16_t use_2gfsk[] = {0x11, 0x20, 0x01, 0x00, 0x0B};
	Si446x_write(use_2gfsk, 5);

	// Set AFSK filter
	uint8_t coeff[] = {0x81, 0x9f, 0xc4, 0xee, 0x18, 0x3e, 0x5c, 0x70, 0x76};
	uint8_t i;
	for(i=0; i<sizeof(coeff); i++) {
		uint16_t msg[] = {0x11, 0x20, 0x01, 0x17-i, coeff[i]};
		Si446x_write(msg, 5);
	}
}

void setPowerLevel(uint8_t level) {
	// Set the Power
	uint16_t set_pa_pwr_lvl_property_command[] = {0x11, 0x22, 0x01, 0x01, level};
	Si446x_write(set_pa_pwr_lvl_property_command, 5);
}

void startTx(void) {
	uint16_t change_state_command[] = {0x34, 0x07};
	Si446x_write(change_state_command, 2);
}

void stopTx(void) {
	uint16_t change_state_command[] = {0x34, 0x03};
	Si446x_write(change_state_command, 2);
}

void radioShutdown(void) {
	RADIO_SDN_SET(true);	// Power down chip
	SPI_DeInit();			// Power down SPI
}

/**
 * Tunes the radio and activates transmission.
 * @param frequency Transmission frequency in Hz
 * @param shift Shift of FSK in Hz
 * @param level Transmission power level (see power level description in config file)
 */
void radioTune(uint32_t frequency, uint8_t level) {
	stopTx();

	if(frequency < 119000000UL || frequency > 1050000000UL)
		frequency = 145300000UL;

	sendFrequencyToSi446x(frequency);	// Frequency
	setPowerLevel(level);				// Power level

	startTx();
}

void setGPIO(bool s) {
	RF_GPIO_SET(s);
}

int8_t Si446x_getTemperature(void) {
	uint16_t txData[2] = {0x14, 0x10};
	uint16_t rxData[7];
	Si446x_read(txData, sizeof(txData), rxData, sizeof(rxData));
	uint16_t adc = rxData[6] | ((rxData[5] & 0x7) << 8);
	return (899*adc)/4096 - 293;
}

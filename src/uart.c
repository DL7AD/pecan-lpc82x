/*
 * @brief UART API in ROM (USART API ROM) polling example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"
#include <string.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Use a buffer size larger than the expected return value of
   uart_get_mem_size() for the static UART handle type */
static uint32_t uartHandleMEM[128];

/* Turn on LED to indicate an error */
static void errorUART(void)
{

}

/* Setup UART handle and parameters */
UART_HANDLE_T* setupUART(void)
{
	uint32_t frg_mult;

	Chip_Clock_SetUARTClockDiv(1);	/* divided by 1 */

	/* Connect the U0_TXD_O and U0_RXD_I signals to port pins(P0.14, P0.0) */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I1);
	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 14);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 0);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	Chip_UART_Init(LPC_USART0);

	/* 9.6KBPS, 8N1, ASYNC mode, no errors, clock filled in later */
	UART_CONFIG_T cfg = {
		0,				/* U_PCLK frequency in Hz */
		9600,			/* Baud Rate in Hz */
		1,				/* 8N1 */
		0,				/* Asynchronous Mode */
		NO_ERR_EN		/* Enable No Errors */
	};

	/* Perform a sanity check on the storage allocation */
	if (LPC_UARTD_API->uart_get_mem_size() > sizeof(uartHandleMEM)) {
		/* Example only: this should never happen and probably isn't needed for
		   most UART code. */
		errorUART();
	}

	/* Setup the UART handle */
	UART_HANDLE_T *uartHandle;
	uartHandle = LPC_UARTD_API->uart_setup((uint32_t) LPC_USART0, (uint8_t *) &uartHandleMEM);
	if (uartHandle == NULL) {
		errorUART();
	}

	/* Need to tell UART ROM API function the current UART peripheral clock
	     speed */
	cfg.sys_clk_in_hz = Chip_Clock_GetSystemClockRate();

	/* Initialize the UART with the configuration parameters */
	frg_mult = LPC_UARTD_API->uart_init(uartHandle, &cfg);
	if (frg_mult) {
		Chip_SYSCTL_SetUSARTFRGDivider(0xFF);	/* value 0xFF should be always used */
		Chip_SYSCTL_SetUSARTFRGMultiplier(frg_mult);
	}

	return uartHandle;
}

/* Send a string on the UART terminated by a NULL character using
   polling mode. */
void putLineUART(UART_HANDLE_T *uartHandle, const char *send_data)
{
	UART_PARAM_T param;

	param.buffer = (uint8_t *) send_data;
	param.size = strlen(send_data);

	/* Polling mode, do not append CR/LF to sent data */
	param.transfer_mode = TX_MODE_SZERO;
	param.driver_mode = DRIVER_MODE_POLLING;

	/* Transmit the data */
	if (LPC_UARTD_API->uart_put_line(uartHandle, &param)) {
		errorUART();
	}
}

/* Receive a string on the UART terminated by a LF character using
   polling mode. */
void getLineUART(UART_HANDLE_T *uartHandle, char *receive_buffer, uint32_t length)
{
	UART_PARAM_T param;

	param.buffer = (uint8_t *) receive_buffer;
	param.size = length;

	/* Receive data up to the CR/LF character in polling mode. Will
	   truncate at length if too long.	*/
	param.transfer_mode = RX_MODE_CRLF_RECVD;
	param.driver_mode = DRIVER_MODE_POLLING;

	/* Receive the data */
	if (LPC_UARTD_API->uart_get_line(uartHandle, &param)) {
		errorUART();
	}
}

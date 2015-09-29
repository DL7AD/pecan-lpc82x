/**************************************************************************//**
 * @file		uart.c
 * @brief		UART Funktionalität
 *
 * @authors		A. Quade
 *
 * @par
 * Projekt:		Indoor_Transmitter
 *
 * @copyright
 * navtec GmbH, Berlin, Germany, www.navtec.de			\n
 * Tel.: +49 (0)3375 / 2465078							\n
 * FAX : +49 (0)3375 / 2465079							\n
 *
 * @since		V1.00
 *
  * @page Peripherals
 * ---
 * UART
 * ------
 * UART RXD: P0.0										\n
 * UART TXD: P0.4 (P0.7)								\n
 *
 *****************************************************************************/
/** @addtogroup System *******************************************************/
/*@{*/
#include "fifo.h"
#include "uart.h"
#include "LPC8xx.h"
#include "core_cm0plus.h"

/****** exportierte Objekte **************************************************/

/****** lokale Funktionen ****************************************************/
#define UART_PRESCALE				1					//!< UART Prescaler
#define UART_FRACTIONAL_M			77					//!< Zähler für fractional divider
// --> U_PCLK = Fcclk / UART_PRESCALE / (1 + UART_FRACTIONAL_M/255) = 48MHz / 1 / (1 + 77/255) = 36.86746988MHz
#define UART_BAUDRATE_VAL_9600		239					//!< Wert für USART Baud Rate Generator register fuer 9k6
#define UART_BAUDRATE_VAL_115200	19					//!< Wert für USART Baud Rate Generator register fuer 115k2
#define UART_BAUDRATE_VAL	UART_BAUDRATE_VAL_9600
// --> Baudrate = U_PCLK / (1 + UART_BAUDRATE_VAL) / 16 = 36.86746988MHz / (1 + 239) / 16 = 9600Hz
// --> Baudrate = U_PCLK / (1 + UART_BAUDRATE_VAL) / 16 = 36.86746988MHz / (1 +  19) / 16 = 115211Hz

static uint32_t crit = 0;

// Es wird UART0 benutzt
#define UART_CFG					LPC_USART0->CFG
#define UART_CTRL					LPC_USART0->CTRL
#define UART_STAT					LPC_USART0->STAT
#define UART_INTENSET				LPC_USART0->INTENSET
#define UART_INTENCLR				LPC_USART0->INTENCLR
#define UART_RXDATA					LPC_USART0->RXDATA
#define UART_RXDATA_STAT			LPC_USART0->RXDATA_STAT
#define UART_TXDATA					LPC_USART0->TXDATA
#define UART_BRG					LPC_USART0->BRG
#define UART_INTSTAT				LPC_USART0->INTSTAT
#define UART_IRQ					UART0_IRQn
#define UART_RESETCTRL_BIT			(1 <<  3)			//!< Bit für UART0 im Peripheral reset control register
#define UART_CLKCTRL_BIT			(1 << 14)			//!< Bit für UART0 im System clock control register
#define INT_PRIORITY_UART			10


#define UART_CLEAR_IR() 			{	\
		\
}

#define UART_ISIRQ_RXREADY()		((UART_STAT & (1 <<  0)) ? true : false)	// Receiver Ready flag
#define UART_ISIRQ_TXREADY()		((UART_STAT & (1 <<  2)) ? true : false)	// Transmitter Ready flag
#define UART_ISIRQ_TXIDLE()			((UART_STAT & (1 <<  3)) ? true : false)	// Transmitter Idle flag
#define UART_DISABLEIRQ_TXREADY()	{UART_INTENCLR = (1 <<  2);}
#define UART_DISABLEIRQ_TXIDLE()	{UART_INTENCLR = (1 <<  3);}
#define UART_START_TX()				{	\
		UART_INTENSET 			= 0x0D;					/* RxReady + TxReady + TxIdle IRQs aktivieren */	\
		NVIC_SetPendingIRQ(UART_IRQ);	\
}

/****** lokale Variablen *****************************************************/
static uint8_t UART_Tx_FIFOBuffer[UART_BUFFERSIZE_TX];	//!< Buffer für Transmit-FIFO
static uint8_t UART_Rx_FIFOBuffer[UART_BUFFERSIZE_RX];	//!< Buffer für Receive-FIFO
static T_ByteFIFO UART_TxFIFO;							//!< Transmit-FIFO
static T_ByteFIFO UART_RxFIFO;							//!< Receive-FIFO

/** @addtogroup ISRHandler **************************************************/
/*@{*/
/**************************************************************************//**
 * @brief		Handler für UART Interrupt
 */
void	On_UART(void)
{
uint32_t Data;

	NVIC_ClearPendingIRQ(UART_IRQ);						// Clear Soft Interrupt

	// Byte empfangen ? ///////////////////////////////////////////////////////
	if (UART_ISIRQ_RXREADY())
	{
		Data = UART_RXDATA_STAT;
		// TODO: Framingerror, Parityerror behandeln
		FIFO_Put(&UART_RxFIFO, (uint8_t)Data);
	}

	// Sendebereit ? //////////////////////////////////////////////////////////
	if (UART_ISIRQ_TXREADY())
	{
		if (FIFO_Get(&UART_TxFIFO, (uint8_t*)&Data))
		{
			UART_TXDATA = (uint8_t)Data;
		}
		else
		{
			if (UART_ISIRQ_TXIDLE())
				UART_DISABLEIRQ_TXIDLE();

			UART_DISABLEIRQ_TXREADY();
		}
	}
}
/*@}*/ /* end of group ISRHandler */

/**************************************************************************//**
* @brief		diverse Initialisierungen
*
* @return      !=0 Alles OK
*/
bool	UART_Init(void)
{
bool result = true;										// Optimist


	if (!FIFO_Init(&UART_TxFIFO, sizeof(UART_Tx_FIFOBuffer), UART_Tx_FIFOBuffer))
		result = false;

	if (!FIFO_Init(&UART_RxFIFO, sizeof(UART_Rx_FIFOBuffer), UART_Rx_FIFOBuffer))
		result = false;

	LPC_SYSCON->SYSAHBCLKCTRL	|= (1 <<  7);		/* Enables clock for SWM */
	LPC_SWM->PINENABLE0 |= (1 <<  0);				/* ACMP_I1	on pin PIO0_0 disabled */
	/*LPC_SWM->PINENABLE0 |= (1 << 24);*/				/* ADC_11	on pin PIO0_4 disabled */
	LPC_SWM->PINENABLE0 |= (1 << 13);				/* ADC_0	on pin PIO0_7 disabled */
	/*LPC_SWM->PINASSIGN0	= (LPC_SWM->PINASSIGN0 & 0xFFFFFF00) | ( 4 <<  0);*/	/* P0_4 ist U0_TXD */
	LPC_SWM->PINASSIGN0	= (LPC_SWM->PINASSIGN0 & 0xFFFFFF00) | ( 7 <<  0);	/* P0_7 ist U0_TXD */
	LPC_SWM->PINASSIGN0	= (LPC_SWM->PINASSIGN0 & 0xFFFF00FF) | ( 0 <<  8);	/* P0_0 ist U0_RXD */
	LPC_SYSCON->SYSAHBCLKCTRL	&=~(1 <<  7);		/* Disables clock for SWM */

	LPC_SYSCON->SYSAHBCLKCTRL |= UART_CLKCTRL_BIT;	/* Enables clock for UART */
	LPC_SYSCON->PRESETCTRL	|= UART_RESETCTRL_BIT;	/* UART nicht mehr RESET */
	NVIC_DisableIRQ(UART_IRQ);						/* IRQ disable und löschen */
	NVIC_ClearPendingIRQ(UART_IRQ);
	UART_INTENSET 			= 0x00;					/* IRQs deaktivieren */

	LPC_SYSCON->UARTCLKDIV	= UART_PRESCALE;		/* UART Prescaler */
	LPC_SYSCON->UARTFRGMULT	= UART_FRACTIONAL_M;	/* Zähler für fractional divider */
	LPC_SYSCON->UARTFRGDIV	= 0xFF;					/* Nenner für fractional divider, muss fest auf 255 */
	UART_BRG				= UART_BAUDRATE_VAL;	/* Baudrate einstellen */
													/*  3             2                  1                  */
													/* 10987654 3 2 1 0 9 8 76 5 4 3 2 1 0 9 87 6 54 32 1 0 */
	UART_CFG				= 0x00000004;			/* 00000000 0 0 0 0 0 0 00 0 0 0 0 0 0 0 00 0 00 01 0 0 : UART 8N1, disabled */
	UART_CTRL				= 0;					/* Nix */
	NVIC_SetPriority(UART_IRQ, INT_PRIORITY_UART);	/* Interrupt einrichten */
	NVIC_EnableIRQ(UART_IRQ);
	UART_INTENSET 			= 0x01;					/* RxReady IRQ aktivieren */
	UART_CFG				|= 1;					/* Enable */

	return result;
}
/**************************************************************************//**
 * @brief		Legt Datenbyte in FIFO, löst Senden aus
 *
 * @param[in]	Data	- Datenbyte
 *
 * @return      true 	- OK, false - FIFO voll
 */
bool	UART_TxByte(uint8_t Data)
{
bool result;

	EnterCritical();
		result = FIFO_Put(&UART_TxFIFO, Data);
		UART_START_TX();
	ExitCritical();

	return result;
}
/**************************************************************************//**
 * @brief		Holt ein evtl. vorhandenes Zeichen
 *
 * @param[out]	pData
 *
 * @return      false - kein Zeichen vorhanden
 */
bool	UART_RxByte(uint8_t* pData)
{
bool result = false;

	if (FIFO_Available(&UART_RxFIFO))
	{
		EnterCritical();
			result = FIFO_Get(&UART_RxFIFO, pData);
		ExitCritical();
	}

	return result;
}
/**************************************************************************//**
 * @brief		Schreibt String in FIFO, löst Senden aus
 * @param[in]	pData	- String
 * @param		Len		- max. Anzahl auszugebener Zeichen
 * @return      Anzahl tatsächlich geschriebener Zeichen
 */
uint32_t	UART_TxString(const uint8_t* pData, uint32_t Len)
{
bool result = true;
uint32_t Count = Len;
const uint8_t	*pB = pData;

	EnterCritical();
		while (	result && (*pB) && (Len))
		{
			if ((result = FIFO_Put(&UART_TxFIFO, *pB++)))
				Len--;
		}
	ExitCritical();

	UART_START_TX();

	return Count - Len;
}

void EnterCritical(void) {
	while(crit);
	crit++;
}

void ExitCritical(void) {
	crit--;
}

/*@}*/ /* end of group System */

/**************************************************************************//**
 * @file		uart.h
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
 *****************************************************************************/
/** @addtogroup System *******************************************************/
/*@{*/
#ifndef __UART__H__
#define __UART__H__

#define UART_BUFFERSIZE_TX			512					//!< Buffer für Transmit-FIFO
#define UART_BUFFERSIZE_RX			512					//!< Buffer für Receive-FIFO

bool	UART_Init(void);

void UART0_IRQHandler(void);
bool UART_TxByte(uint8_t Data);
bool UART_RxByte(uint8_t* pData);
uint32_t UART_TxString(const uint8_t* pData, uint32_t Len);
void EnterCritical(void);
void ExitCritical(void);

/*@}*/ /* end of group System */
#endif /* __UART__H__ */

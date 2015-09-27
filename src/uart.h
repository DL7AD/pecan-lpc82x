#ifndef __UART_H__
#define __UART_H__

#include <chip.h>

UART_HANDLE_T* UART_Init(uint32_t baudrate);
void UART_DeInit(UART_HANDLE_T* uartHandle);
void putLineUART(UART_HANDLE_T *uartHandle, const uint8_t *send_data);
void getLineUART(UART_HANDLE_T *uartHandle, uint8_t *receive_buffer, uint32_t length);

#endif

#ifndef __UART_H__
#define __UART_H__

#include <chip.h>

UART_HANDLE_T* setupUART(void);
void putLineUART(UART_HANDLE_T *uartHandle, const char *send_data);
void getLineUART(UART_HANDLE_T *uartHandle, char *receive_buffer, uint32_t length);

#endif

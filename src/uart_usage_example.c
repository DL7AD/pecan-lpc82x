#include "chip.h"
#include <cr_section_macros.h>
#include "uart.h"
#include <string.h>

#define RECV_BUFF_SIZE 128
static char recv_buf[RECV_BUFF_SIZE];

/* ASCII code for escapre key */
#define ESCKEY 27

/* UART handle and memory for ROM API */
static UART_HANDLE_T *uartHandle;


/*int main(void)
{


}*/


int uart_example(void)
{
	SystemCoreClockUpdate();
	Chip_GPIO_Init(LPC_GPIO_PORT);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Allocate UART handle, setup UART parameters, and initialize UART
	   clocking */
	uartHandle = setupUART();

	/* Transmit the welcome message and instructions using the
	   putline function */
	putLineUART(uartHandle, "LPC8XX USART API ROM polling Example\r\n");
	putLineUART(uartHandle, "Enter a string, press enter (CR+LF) to echo it back:\r\n");

	/* Get a string for the UART and echo it back to the caller. Data is NOT
	   echoed back via the UART using this function. */
	getLineUART(uartHandle, recv_buf, sizeof(recv_buf));
	recv_buf[sizeof(recv_buf) - 1] = '\0';	/* Safety */
	if (strlen(recv_buf) == (sizeof(recv_buf) - 1)) {
		putLineUART(uartHandle, "**String was truncated, input data longer than "
					"receive buffer***\r\n");
	}
	putLineUART(uartHandle, recv_buf);

	/* Transmit the message for byte/character part of the exampel */
	putLineUART(uartHandle, "\r\nByte receive with echo: "
				"Press a key to echo it back. Press ESC to exit\r");

	/* Endless loop until ESC key is pressed */
	recv_buf[0] = '\n';
	while (recv_buf[0] != ESCKEY) {
		/* Echo it back */
		putLineUART(uartHandle, recv_buf);

		/* uart_get_char will block until a character is received */
		recv_buf[3] = '\0';
		recv_buf[4] = '\0';
		recv_buf[5] = '\0';
		while(recv_buf[3] != 'G' || recv_buf[4] != 'G' || recv_buf[5] != 'A') {
			getLineUART(uartHandle, recv_buf, 128);
		}
	}

	/* Transmit the message for byte/character part of the exampel */
	putLineUART(uartHandle, "\r\nESC key received, exiting\r\n");

	return 0;
}

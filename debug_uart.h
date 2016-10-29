/*
 * debug_uart.h
 *
 *  Created on: 26 Nov 2014
 *      Author: mrowsell
 */

#ifndef DEBUG_UART_H_
#define DEBUG_UART_H_

char out[] = { "The UART works, baby!\n" };
char *pOut = out;

void uart_puts(char *c, int length) {
	int i;
	for(i = 0; (*c != '\0') || i < length; i++) {
		while(!(UCA1IFG & UCTXIFG));
		UCA1TXBUF = *c;
		c++;
	}
}

void uart_putc(char c) {
	while(!(UCA1IFG & UCTXIFG));
	UCA1TXBUF = c;
}

void uart_newl(void) {
	while(!(UCA1IFG & UCTXIFG));
	UCA1TXBUF = 0x0A;
}

#endif /* DEBUG_UART_H_ */

/*
 * Test platform for LCD control via 74HC595
 *
 *  DL: Data Length -- 0, 4-bit, 1, 8-bit
 *  N: 0, 1 line, 1, 2 lines
 *  F: 0, 5x7 dots, 1, 5x10 dots
 *  SC: 0, move cursor, 1, shift display
 *  RL: 0, shift left, 1, shift right
 *  D: 0, display off, 1, display on
 *  C: 0, cursor off, 1, cursor on
 *  B: 0, blink off, 1, blink on
 *  ID: 0, decrement cursor position, 1, increment cursor position
 *  S: 0, no display shift, 1, display shift
 *
 *  Also, the 74HC595 is attached thus:
 *  7   6   5   4   3   2   1   0
 *  D7  D6  D5  D4  EN  RW  RS  LED
 *
 *  Created on: 20 Nov 2014
 *      Author: mrowsell
 */

#ifndef LCD44780_H_
#define LCD44780_H_

enum {
	homeinstruction = 0x01,
	cursorhome = 0x02,
	entrymode = 0x04,
	displayonoff = 0x08,
	cursorshift = 0x10,
	functionset = 0x20,
	ddraddress = 0x80
};

typedef struct lcd {
	char lcdout;
	char line_one[20];
	char line_two[20];
	unsigned int position;
	_Bool updated;
} LCD_t;

void LCDput(LCD_t *temp) {
	char tempdata = 0x00;

	// First nibble
	tempdata = temp->lcdout & 0xF0;
	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = (tempdata | 0x08);
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	// Second nibble
	tempdata = (temp->lcdout & 0x0F) << 4;
	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = (tempdata | 0x08);
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);
	P3OUT &= ~(0x04);
	_delay_cycles(1000);
}

void LCDcharset(LCD_t *temp) {
	// The difference between this and the LCDput function
	// is that this puts RS high the whole time, so that
	// characters can be written to the display

	char tempdata = 0x00;

	// First nibble
	tempdata = temp->lcdout & 0xF0;
	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata | 0x02;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = (tempdata | 0x0A);
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata | 0x02;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	// Second nibble
	tempdata = (temp->lcdout & 0x0F) << 4;
	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata | 0x02;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = (tempdata | 0x0A);
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = tempdata | 0x02;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);
	P3OUT &= ~(0x04);
	_delay_cycles(1000);
}

void LCDset(LCD_t *temp) {

	temp->lcdout = (homeinstruction); // clear screen and move cursor to home position
	LCDput(temp);
	_delay_cycles(50000);

	uint_fast8_t charPosition = 0;	// this represents where we are on each line
	uint_fast8_t ddrPosition = 0;	// only needs to change once, to switch lines as
									// the cursor auto-increments for us

	for(charPosition = 0; charPosition < 0x10; charPosition++) {
		temp->lcdout = temp->line_one[charPosition];
		if(temp->lcdout == 0x00)
			temp->lcdout = 0x20;
		LCDcharset(temp);
	}
	ddrPosition = 0x40; // move to second line
	temp->lcdout = (ddraddress | ddrPosition); // move cursor to second line
	LCDput(temp);

	for(charPosition = 0; charPosition < 0x10; charPosition++) {
		temp->lcdout = temp->line_two[charPosition];
		if(temp->lcdout == 0x00)
			temp->lcdout = 0x20;
		LCDcharset(temp);
	}
	temp->updated = true;
}

void LCDsetup(LCD_t *temp) {
	temp->lcdout = (functionset); // 4-bit
	LCDput(temp);
	temp->lcdout = (functionset | 0x08); // 4-bit, 2 lines, 5x7 dots
	LCDput(temp);
	temp->lcdout = (entrymode | 0x02); // cursor increment
	LCDput(temp);
	temp->lcdout = (displayonoff | 0x04); // display on, cursor off, blink off
	LCDput(temp);
	temp->lcdout = (cursorhome);
	LCDput(temp);
	_delay_cycles(50000);
	temp->lcdout = (homeinstruction);
	LCDput(temp);
	_delay_cycles(50000);
	temp->updated = true;
}

void onLED(void) {

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = 0x01;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);
}

void offLED(void) {

	P3OUT &= ~(0x04);
	_delay_cycles(750);
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = 0x00;
	while(!(UCA0IFG & UCRXIFG));
	_delay_cycles(750);
	P3OUT |= 0x04;
	_delay_cycles(200);
}

#endif /* LCD44780_H_ */

#include <msp430.h>
#include <intrinsics.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lcd44780.h"
#include "ds18b20.h"

extern LCD_t dispLCD = {
		0x00,					// current output instruction/data
		" ",
		" ",					// 32 blank characters to start
		0x00,					// character postion
		false					// updated since last change
},
*pDispLCD = &dispLCD;

extern DS18B20_t tempSensor = {
		0,		// readData
		0,		// writeData
		0,		// romCode
		0,		// dataSize
		false,	// updated
		"null"	// thermData[14]
},
		*pTempSensor = &tempSensor;

/*
char out[] = { "The UART works, baby!\n" };
char *pOut = out;

void uart_puts(char *c, int length) {
	int i;
	for(i = 0; i < length; i++) {
		while(!(UCA1IFG & UCTXIFG));
		UCA1TXBUF = *c + i;
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
 */
void systemInit(void) {

	WDTCTL = WDTPW | WDTHOLD;				// Stop watchdog timer

	//------- VCORE RESET SECTION -------//
	PMMCTL0_H = 0xA5; 						// Open PMM module registers for write access
	PMMCTL0 = 0xA500 + PMMCOREV_3; 			// Set VCore to 1.85 V
	SVSMLCTL = SVMLE + SVSMLRRL_6; 			// Set SVM new Level
	while ((PMMIFG & SVSMLDLYIFG) == 0); 	// Wait till SVM is settled (Delay)
	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG); 		// Clear already set flags
	if ((PMMIFG & SVMLIFG))	;				// Wait till level is reached
	//while ((PMMIFG & SVMLVLRIFG) == 0);
	SVSMLCTL &= ~SVMLE;						// Disable Low side SVM
	PMMCTL0_H = 0x00;						// Lock PMM module registers for write access
	//------- END VCORE RESET SECTION -------//

	//------- CLOCK CONFIG SECTION -------//
	P1DIR = 0x01;
	P1SEL = 0x01;
	P5SEL |= 0x0C;					// set XT2 crystal pins as clock pins, not I/O
	UCSCTL3 = SELREF__XT2CLK;		// select XT2 as FLL reference clock
	UCSCTL1 = DCORSEL_6;			// select proper DCO tap based on expected freq, below
	UCSCTL2 = FLLD__2 + FLLN1;		// times 2, times 3. With XT2 at 4MHz, DCOCLK will be
									// 24MHz, and DCOCLKDIV will be 12MHz
	UCSCTL4 = SELA__DCOCLKDIV + SELS__DCOCLKDIV + SELM__DCOCLK;
									// set ACLK to XT2 (4MHz), MCLK to DCOCLK (24MHz),
									// SMCLK to DCOCLKDIV (12MHz)
	UCSCTL5 = DIVS__2 + DIVA__4;	// Divide SMCLK down to 6MHz
	//------- END CLOCK CONFIG -------//

	//------- SPI CONFIG -------//
	// P2.7 = UCA0CLK, P3.2 = CS, P3.3 = UCA0SIMO, P3.4 = UCA0SOMI
	P2SEL |= 0x80; 			// P2.7 Peripheral Control
	P3SEL |= 0x18; 			// P3.3,3.4 Peripheral Control
	P3DIR |= 0x04; 			// P3.2 output
	P3OUT &= ~0x04; 		// set CS low

	UCA0CTL1 = UCSWRST; 							// enable USCI reset mode, for safety
	UCA0CTL0 = UCMST + UCMSB + UCSYNC + UCCKPH; 	// master mode, 3-pin SPI (synchronous), MSB first, no STE
	UCA0CTL1 = UCSSEL0; 							// BRCLK from ACLK
	//UCA0BR0 |= 0x03; 								// divide BRCLK by 4 (3+1) (1MHz?)
	//UCA0BR1 = 0;
	UCA0CTL1 &= ~UCSWRST; 	// enable USCI

	//-------END SPI CONFIG -------//

	//------- UART CONFIG -------//
	// UART pins UCA1TXD(P4.4) and UCA1RXD(P4.5) are connected through the ezFET debug interface
	UCA1CTL1 = UCSWRST;		// enter USCI reset mode, for safety

	PMAPKEYID = PMAPKEY; 		// unlock port map control register
	PMAPCTL = PMAPRECFG;		// allow multiple reconfigurations
	P4MAP4 = PM_UCA1TXD;		// set 4.4 as UART TX
	P4MAP5 = PM_UCA1RXD;		// set 4.5 as UART RX
	PMAPKEYID = 0x034F1;		// write bad key to close lock
	P4SEL |= 30; 				// set 4.4 & 4.5 to peripheral control
	UCA1CTL1 = UCSSEL__ACLK;	// choose ACLK (4MHz)
	UCA1BRW = 208;				// Baud rate settings for 19200 baud
	UCA1MCTL = UCBRS0 + UCBRS1;
	UCA1CTL1 &= ~UCSWRST;		// release USCI reset
	UCA1IE = UCRXIE;
	//------- END UART -------//

	//------- BUTTON SETUP -------//
	// Buttons S2 and S1 are on pins P1.1 and P2.1 respectively
	P2REN |= 0x02; // P2.1 resistor enable
	P2OUT |= 0x02; // P2.1 pullup on

	P2IES |= 0x02; //falling edge interrupt P2.1
	P2IFG &= ~(0x02);
	P2IE |= 0x02; // interrupt enabled, P2.1
	//------- END BUTTON -------//
}

void main() {

	systemInit();

	P4DIR |= 0x80;
	TA0CTL = TASSEL_1 + MC_2 + ID_3 + TACLR + TAIE;  // ACLK, contmode, clear TAR
	                                            	// enable interrupt
	offLED();
	LCDsetup(pDispLCD);
	if(pDispLCD->updated) ;
	sprintf(pDispLCD->line_one, "Testing 1 2 3");
	sprintf(pDispLCD->line_two, "Aww hell yes!!");
	pDispLCD->position = 0;
	pDispLCD->updated = false;
	LCDset(pDispLCD);
	if(pDispLCD->updated)
		onLED();

	__bis_SR_register(LPM1_bits + GIE);	// enable GIE and enter LPM1
	__no_operation();

	// LOOP SECTION -----D-E-B-U-G--O-N-L-Y-----
	while(1) {
		__no_operation();
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) PORT2_ISR (void)
#else
#error Compiler not supported!
#endif
{

	sprintf(pDispLCD->line_one, "Switched!       ");
	pDispLCD->updated = false;
	P2IE &= ~(0x02); //disable this interrupt
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER0_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
	switch(__even_in_range(TA0IV,14))
	  {
	    case 14:
	    		if(!pDispLCD->updated)	{	// overflow
	    			pDispLCD->position = 0;
	    			LCDset(pDispLCD);
	    			P2IE |= 0x02;
	    		}
	             break;
	    default: break;
	  }
}





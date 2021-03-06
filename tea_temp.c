#include <msp430.h>
#include <intrinsics.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lcd44780.h"
#include "ds18b20.h"
#include "debug_uart.h"

extern LCD_t dispLCD = {
		0x00,					// current output instruction/data
		" ",
		" ",					// 32 blank characters to start
		0x00,					// character postion
		false					// updated since last change
},
*pDispLCD = &dispLCD;

extern DS18B20_t tempSensor = {
		0,		// readData uint16_t
		0,		// writeData uint32_t
		0,		// romCode uint64_t
		0,		// dataSize char
		false,	// updated _Bool
		"null",	// thermData char[14]
		0		// CRC char
},
		*pTempSensor = &tempSensor;

char temp[64];

void systemInit(void) {

	WDTCTL = WDTPW | WDTHOLD;				// Stop watchdog timer

	//------- VCORE RESET SECTION -------//
	PMMCTL0_H = 0xA5; 						// Open PMM module registers for write access
	PMMCTL0 = 0xA500 + PMMCOREV_3; 			// Set VCore to 1.85 V
	SVSMLCTL = SVMLE + SVSMLRRL_6 + SVSLFP + \
				SVSLRVL_3 + SVSLE + SVMLFP;	// Enable Vcore supply monitor, level 6, high performance mode,
											// and Vcore supply supervisor, level 3, high performance mode.
	SVSMHCTL = SVSHE + SVSHFP + SVSHRVL_3 + \
				SVSMHRRL_3;					// Enable DVcc supply supervisor, level 3, high performace
	while ((PMMIFG & SVSMLDLYIFG) == 0); 	// Wait for Vcore delay element to expire
	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG); 		// Clear already set flags
	if ((PMMIFG & SVMLIFG))					// If Vcore still below target, wait for it to rise
		while ((PMMIFG & SVMLVLRIFG) == 0);
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
	UCSCTL4 = SELA__REFOCLK + SELS__DCOCLKDIV + SELM__DCOCLK;
									// set ACLK to REFOCLK (32.768kHz), MCLK to DCOCLK (24MHz),
									// SMCLK to DCOCLKDIV (12MHz)
	UCSCTL5 = DIVS__4;	// Divide SMCLK down to 3MHz
	//------- END CLOCK CONFIG -------//

	//------- SPI CONFIG -------//
	// P2.7 = UCA0CLK, P3.2 = CS, P3.3 = UCA0SIMO, P3.4 = UCA0SOMI
	P2SEL |= 0x80; 			// P2.7 Peripheral Control
	P3SEL |= 0x18; 			// P3.3,3.4 Peripheral Control
	P3DIR |= 0x04; 			// P3.2 output
	P3OUT &= ~0x04; 		// set CS low

	UCA0CTL1 = UCSWRST; 							// enable USCI reset mode, for safety
	UCA0CTL0 = UCMST + UCMSB + UCSYNC + UCCKPH; 	// master mode, 3-pin SPI (synchronous), MSB first, no STE
	UCA0CTL1 = UCSSEL__SMCLK; 							// BRCLK from ACLK
	//UCA0BR0 |= 0x03; 								// divide BRCLK by 4 (3+1) (1MHz?)
	UCA0CTL1 &= ~UCSWRST; 	// enable USCI

	//-------END SPI CONFIG -------//

	//------- UART CONFIG -------//
	// UART pins UCA1TXD(P4.4) and UCA1RXD(P4.5) are connected through the ezFET debug interface
	UCA1CTL1 = UCSWRST;		// enter USCI reset mode, for safety

	PMAPKEYID = PMAPKEY; 		// unlock port map control register
	PMAPCTL = PMAPRECFG;		// allow multiple reconfigurations
	P4MAP4 = PM_UCA1TXD;		// set 4.4 as UART TX
	P4MAP5 = PM_UCA1RXD;		// set 4.5 as UART RX
	PMAPKEYID = 0x34F1;			// write bad key to close lock
	P4SEL |= 0x30; 				// set 4.4 & 4.5 to peripheral control
	UCA1CTL1 = UCSSEL__SMCLK;	// choose ACLK (3MHz)
	UCA1BRW = 156;				// Baud rate settings for 19200 baud
	UCA1MCTL = UCBRS1;
	UCA1CTL1 &= ~UCSWRST;		// release USCI reset
	UCA1IE = UCRXIE;
	//------- END UART -------//

	//------- BUTTON SETUP -------//
	// Buttons S2 and S1 are on pins P1.1 and P2.1 respectively
//	P2REN |= 0x02; // P2.1 resistor enable
//	P2OUT |= 0x02; // P2.1 pullup on
//
//	P2IES |= 0x02; //falling edge interrupt P2.1
//	P2IFG &= ~(0x02);
//	P2IE |= 0x02; // interrupt enabled, P2.1
	//------- END BUTTON -------//
}

void main() {

	systemInit(); // MCLK is 24MHz, SMCLK is 6MHz, ACLK is 1MHz

	P4DIR |= 0x80;
	P7DIR |= 0x10; // P7.4 is our profiling pin
	TA0CTL = TASSEL__ACLK + MC_2 + TACLR + TAIE;  // ACLK, contmode, clear TAR, 1/(32768/65535) = ~2s
	                                            	 // enable interrupt

	//------- INITIALIZE LCD -------//
	offLED();
	LCDsetup(pDispLCD);
	if(pDispLCD->updated) ;
	snprintf(pDispLCD->line_one, 17, "...WARMING UP...");
	snprintf(pDispLCD->line_two, 17, "Genmaicha T 0:30");
	pDispLCD->position = 0;
	pDispLCD->updated = false;
	LCDset(pDispLCD);
	if(pDispLCD->updated)
		onLED();
	//------- END LCD INIT -------//

	checkTempSensor(pTempSensor);

	getTemp(pTempSensor);
	sprintf(temp, "Raw temp: %u", pTempSensor->readData);
	uart_puts(temp, 13);
	convThermString(pTempSensor);
	sprintf(temp, "Converted temp: %s", pTempSensor->thermData);
	uart_puts(temp, 24);

	__bis_SR_register(LPM1_bits + GIE);	// enable GIE and enter LPM1
	__no_operation();

	// LOOP SECTION -----D-E-B-U-G--O-N-L-Y-----
	while(1) {
		__no_operation();
	}
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
	    case 14:	// overflow
	    			P7OUT |= 0x10;
	    			getTemp(pTempSensor);
	    			convThermString(pTempSensor);
	    			snprintf(pDispLCD->line_one, 17, "Temp: %s C  ", pTempSensor->thermData);
	    			pDispLCD->position = 0;
	    			LCDset(pDispLCD);
	    			P7OUT &= ~0x10;
	             break;
	    default: break;
	  }
}

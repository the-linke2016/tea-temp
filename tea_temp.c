#include <msp430.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "max7219.h"
#include "ds18b20.h"

extern Max7219_t tempDisplay = { // create a new display structure
		{
				(DIGIT0 | BLANK),
				(DIGIT1 | BLANK),
				(DIGIT2 | BLANK),
				(DIGIT3 | BLANK),
				(DIGIT4 | BLANK),
				(DIGIT5 | BLANK),
				(DIGIT6 | BLANK),
				(DIGIT7 | BLANK), // rest of digits blank
				(INTENSE | LEVEL10),
				(SHUTDWN | OFF),
				(SCANL | SCAN_8),
				(DTEST | TESTOFF),
				(DECMODE | NODECODE),
				(ENDLIST)
		},
		false,	// updated false initially
		"        " //initial string
},
*pTempDisplay = &tempDisplay; // create a pointer to the display

extern DS18B20_t tempSensor = {
		0,		// readData
		0,		// writeData
		0,		// romCode
		0,		// dataSize
		false,	// updated
		"null"	// thermData[14]
},
		*pTempSensor = &tempSensor;

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
	UCSCTL4 = SELA__XT2CLK + SELS__DCOCLKDIV + \
			SELM__DCOCLK;			// set ACLK to XT2 (4MHz), MCLK to DCOCLK (24MHz),
									// SMCLK to DCOCLKDIV (12MHz)
	UCSCTL5 = DIVS__2;	// Divide SMCLK down to 6MHz
	//------- END CLOCK CONFIG -------//

	//------- SPI CONFIG -------//
	// P2.7 = UCA0CLK, P3.2 = CS, P3.3 = UCA0SIMO, P3.4 = UCA0SOMI

	UCA0CTL1 = UCSWRST; 				// enable USCI reset mode, for safety
	UCA0CTL0 = UCMST + UCMSB + UCSYNC; 	// master mode, 3-pin SPI (synchronous), MSB first, no STE
	UCA0CTL1 = UCSSEL0; 				// BRCLK from ACLK
	UCA0BR0 = 0x03; 					// divide BRCLK by 4 (3+1) (1MHz?)

	P3DIR |= 0x04; 			// P3.2 output
	P3OUT |= 0x04; 			// set CS high
	P2SEL |= 0x80; 			// P2.7 Peripheral Control
	P3SEL |= 0x18; 			// P3.3,3.4 Peripheral Control
	UCA0CTL1 &= ~UCSWRST; 	// enable USCI
	UCA0IFG &= ~(UCTXIFG);  // clear flag
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
}
void main() {

	systemInit();

	// set display
	pTempDisplay->updated = setDispString(pTempDisplay);

	if(pTempDisplay->updated == true)
		Serial.println("Display updated!");
	else
		Serial.println("What the hell happened?");

	if(oneInit())
		Serial.println("1-Wire presence pulse detected");
	else
		Serial.println("No presence pulse, damn!");

	// LOOP SECTION -----D-E-B-U-G--O-N-L-Y-----
	while(1) {
		_Bool onewireRead = false;
		oneInit();
		pTempSensor->writeData = 0x00CC;
		pTempSensor->dataSize = 8;
		oneWrite(pTempSensor);
		pTempSensor->writeData = 0x0044;
		oneWrite(pTempSensor);
		delay(750);
		oneInit();
		pTempSensor->writeData = 0x00CC;
		oneWrite(pTempSensor);
		pTempSensor->writeData = 0x00BE;
		oneWrite(pTempSensor);
		pTempSensor->dataSize = 16;
		onewireRead = oneRead(pTempSensor);
		oneInit();
		if(onewireRead) {
			sprintf(pTempDisplay->string, "    %04.2f", convThermString(pTempSensor));
			setDispString(pTempDisplay);
			Serial.print("Data read was: ");
			Serial.println(pTempDisplay->string);
		} else
			Serial.println("Some kind of read error.");
		delay(2000); // wait a second
	}
}





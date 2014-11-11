#include <msp430.h>
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
		0,		// dataSize
		false,	// updated
		"null"	// thermData[14]
},
		*pTempSensor = &tempSensor;

void main() {

	WDTCTL = WDTPW | WDTHOLD;				// Stop watchdog timer

	//------- VCORE RESET SECTION -------//
	PMMCTL0_H = 0xA5; 						// Open PMM module registers for write access
	PMMCTL0 = 0xA500 + PMMCOREV_2; 			// Set VCore to 1.75 V
	SVSMLCTL = SVMLE + SVSMLRRL_6; 			// Set SVM new Level
	while ((PMMIFG & SVSMLDLYIFG) == 0); 	// Wait till SVM is settled (Delay)
	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG); 		// Clear already set flags
	if ((PMMIFG & SVMLIFG))					// Wait till level is reached
		while ((PMMIFG & SVMLVLRIFG) == 0);
	SVSMLCTL &= ~SVMLE;						// Disable Low side SVM
	PMMCTL0_H = 0x00;						// Lock PMM module registers for write access
	//------- END VCORE RESET SECTION --------//

	P5SEL |= 0x0C;					// set XT2 crystal pins as clock pins, not I/O
	UCSCTL1_L = 0x50;
	UCSCTL2 = 0x021F;
	UCSCTL4_L = 0x54;
	UCSCTL4_H = 0x05;
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





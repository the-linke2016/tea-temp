#include <SPI.h>
#include <OneWire.h>
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
	FALSE,	// updated false initially
	"tp      " //initial string
},
*pTempDisplay = &tempDisplay; // create a pointer to the display

void setup() {
	Serial.begin(19200);
	SPI.begin(MAXCS); // CS on 52
	Serial.print("SPI started on ");
	Serial.println(MAXCS, DEC);

	// set display
	pTempDisplay->updated = setDispString(pTempDisplay);

	if(pTempDisplay->updated == TRUE)
		Serial.println("Display updated!");
	else
		Serial.println("What the hell happened?");

	// LOOP SECTION -----D-E-B-U-G--O-N-L-Y-----
	while(1) {
		long int z;
		for(z = 0; z <= 0xFFFFFF; z++) {
			snprintf(pTempDisplay->string, 9, "tP%06X", z);
			//Serial.println(pTempDisplay->string);
			pTempDisplay->updated = FALSE;
			pTempDisplay->updated = setDispString(pTempDisplay);
			if(pTempDisplay->updated == FALSE)
				Serial.println("Error updating display - SPI bus?");
			else
				Serial.println(z);

			delay(25); // wait a second
		}
	}
}



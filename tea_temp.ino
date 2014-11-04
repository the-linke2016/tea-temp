#include <SPI.h>
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
	"tP      " //initial string
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

void setup() {
	Serial.begin(19200);
	SPI.begin(MAXCS); // CS on 52
	Serial.print("SPI started on ");
	Serial.println(MAXCS, DEC);

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
		pTempSensor->writeData = 0x00CC;
		pTempSensor->dataSize = 8;
		oneWrite(pTempSensor);
		delay(250); // wait a second
	}
}




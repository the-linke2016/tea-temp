// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>
#define DQ 22
typedef struct temperatureSense {
	uint16_t readData;
	uint16_t writeData;
	int dataSize;
	_Bool updated;
	char thermData[14];
} DS18B20_t;

_Bool oneInit(void) {
	_Bool presence = FALSE;
	pinMode(DQ, OUTPUT);
	digitalWrite(DQ, LOW);
	delayMicroseconds(500);
	pinMode(DQ, INPUT); // external resistor will pull this up
	delayMicroseconds(90);
	if(!digitalRead(DQ))
		delayMicroseconds(390);
		return TRUE;
	else
		delayMicroseconds(390);
		return FALSE;
}
_Bool oneWrite(DS18B20_t *sensor) {
	pinMode(DQ, OUTPUT);
	int temp;
	int q;
	for(q = 0; q < sensor->dataSize; q++) {
		temp = (sensor->writeData >> q) & 0x0001;
		if(temp) {
			pinMode(DQ, OUTPUT);
			digitalWrite(DQ, LOW);
			delayMicroseconds(5);
			pinMode(DQ, INPUT);
			delayMicroseconds(55);
		}
		else {
			pinMode(DQ, OUTPUT);
			digitalWrite(DQ, LOW);
			delayMicroseconds(120);
			pinMode(DQ, INPUT);
		}
		delayMicroseconds(1);
	}
	return TRUE;
}

uint16_t oneRead(DS18B20_t *sensor) {

}

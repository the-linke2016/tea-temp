// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>

#define DQ 22
typedef struct temperatureSense {
	uint32_t readData;
	uint32_t writeData;
	char dataSize;
	_Bool updated;
	char thermData[14];
} DS18B20_t;

_Bool oneInit(void) {
	_Bool presence = false;
	pinMode(DQ, OUTPUT);
	digitalWrite(DQ, LOW);
	delayMicroseconds(500);
	pinMode(DQ, INPUT); // external resistor will pull this up
	delayMicroseconds(90);
	if(!digitalRead(DQ)) {
		delayMicroseconds(390);
		return true;
	} else {
		delayMicroseconds(390);
		return false;
	}
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
		} else {
			pinMode(DQ, OUTPUT);
			digitalWrite(DQ, LOW);
			delayMicroseconds(120);
			pinMode(DQ, INPUT);
		}
		delayMicroseconds(1);
	}
	return true;
}

_Bool oneRead(DS18B20_t *sensor) {

	int q;
	for(q = 0; q < sensor->dataSize; q++) {
		pinMode(DQ, OUTPUT);
		digitalWrite(DQ, LOW);
		delayMicroseconds(2);
		pinMode(DQ, INPUT);
		delayMicroseconds(9);

		if(digitalRead(DQ)) {
			sensor->readData >> 1;
			sensor->readData |= 0x8000;
		} else {
			sensor->readData >> 1;
		}
	}
	return true;
}



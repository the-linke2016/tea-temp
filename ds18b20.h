// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>

#define DQ 22
typedef struct temperatureSense {
	uint16_t readData;
	uint32_t writeData;
	uint64_t romCode;
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
		delayMicroseconds(10);
	}
	return true;
}

_Bool oneRead(DS18B20_t *sensor) {
	char temp;
	uint16_t datatemp;
	int q;
	for(q = 0; q < sensor->dataSize; q++) {
		pinMode(DQ, OUTPUT);
		digitalWrite(DQ, LOW);
		pinMode(DQ, INPUT);
		delayMicroseconds(8);
		temp = digitalRead(DQ);
		
		if(temp == 1) {
			datatemp |= 0x8000;
			datatemp >> 1;
			
		} else {
			datatemp >> 1;
		}
		delayMicroseconds(60);
	}
	sensor->readData = datatemp;
	return true;
}

float convThermString(DS18B20_t *sensor) {
	uint16_t temp = sensor->readData;
	float whole;
	whole = (float)(temp >> 4);
	temp = sensor->readData;
	
	if(temp & 0x0001)
		whole += 0.0625;
	if(temp & 0x0002)
		whole += 0.125;
	if(temp & 0x0004)
		whole += 0.25;
	if(temp & 0x0008)
		whole += 0.5;
	
	return whole;
	
}



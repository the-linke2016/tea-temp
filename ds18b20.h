// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>

#define _1us	24
#define _5us	120
#define _10us	240
#define _55us	1320
#define _60us	1440
#define _90us	2160
#define _120us	2880
#define _390us	9360
#define _500us	12000

char crctable[256] = {
		0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31,
		65,	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130,
		220, 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60,
		98,	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161,
		255, 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89,
		7, 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196,
		154, 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122,
		36,	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231,
		185, 140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147,
		205, 17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14,
		80,	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176,
		238, 50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45,
		115, 202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213,
		139, 87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72,
		22,	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246,
		168, 116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

typedef struct temperatureSense {
	uint16_t readData;
	uint32_t writeData;
	uint64_t romCode;
	char dataSize;
	_Bool updated;
	char thermData[14];
	char CRC;
} DS18B20_t;

_Bool oneInit(void) {
	P8DIR |= 0x02;
	P8OUT &= ~(0x02);
	_delay_cycles(_500us);
	P8DIR &= ~(0x02); // external resistor will pull this up
	_delay_cycles(_90us);
	if(!(P8IN & 0x02)) {
		_delay_cycles(_390us);
		return true;
	} else {
		_delay_cycles(_390us);
		return false;
	}
}

_Bool oneWrite(DS18B20_t *sensor) {
	P8DIR |= 0x02;
	int temp;
	int q;
	for(q = 0; q < sensor->dataSize; q++) {
		temp = (sensor->writeData >> q) & 0x0001;
		if(temp) {
			P8DIR |= 0x02;
			P8OUT &= ~(0x02);
			_delay_cycles(_5us);
			P8DIR &= ~(0x02);
			_delay_cycles(_55us);
		} else {
			P8DIR |= 0x02;
			P8OUT &= ~(0x02);
			_delay_cycles(_120us);
			P8DIR &= ~(0x02);
		}
		_delay_cycles(_10us);
	}
	return true;
}

_Bool oneRead(DS18B20_t *sensor) {
	uint16_t datatemp;
	int q;
	for(q = sensor->dataSize; q > 0; q--) {
		P8DIR |= 0x02;
		P8OUT &= ~(0x02); // pull low to signal start of bit
		P8DIR &= ~(0x02);
		_delay_cycles(_10us);
		if((P8IN & 0x02)) {
			datatemp |= 0x8000;
			datatemp >>= 1;
		} else {
			datatemp >>= 1;
		}
		_delay_cycles(_60us);
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

void doCRC(DS18B20_t *sensor, char CRCval) {
	sensor->CRC = crctable[(sensor->CRC ^ CRCval)];
}

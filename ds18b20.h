// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>

#define _1us	24
#define _5us	120
#define _6us	144
#define _10us	240
#define _20us	480
#define _50us	1200
#define _60us	1440
#define _64us	1536
#define _90us	2160
#define _390us	9360
#define _500us	12000

#define _1ms 	24000
#define _10ms	240000
#define _200ms	4800000
#define _750ms	18000000

static const uint8_t crctable[256] = {
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
	uint_fast16_t readData;
	uint_fast32_t writeData;
	uint64_t romCode;
	uint_least8_t dataSize;
	_Bool updated;
	char thermData[14];
	uint_least8_t CRC;
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
	uint_fast8_t temp;
	uint_fast8_t q;
	for(q = 0; q < sensor->dataSize; q++) {
		temp = (sensor->writeData >> q) & 0x0001;
		if(temp) {					// send a 1
			P8DIR |= 0x02; 			// output
			P8OUT &= ~(0x02); 		// pull down
			_delay_cycles(_6us);
			P8DIR &= ~(0x02); 		// input/tristate
			_delay_cycles(_64us);
		} else {					// send a 0
			P8DIR |= 0x02; 			// output
			P8OUT &= ~(0x02); 		// pull down
			_delay_cycles(_50us);
			P8DIR &= ~(0x02); 		// input/tristate
			_delay_cycles(_20us);
		}
		_delay_cycles(_5us);		// end of window
	}
	return true;
}

_Bool oneRead(DS18B20_t *sensor) {
	uint_fast16_t datatemp;
	uint_fast8_t q;
	for(q = sensor->dataSize; q > 0; q--) {
		P8DIR |= 0x02;
		P8OUT &= ~(0x02); // pull low to signal start of bit
		_delay_cycles(_6us);
		P8DIR &= ~(0x02); // input/tristate
		_delay_cycles(_6us);
		if((P8IN & 0x02)) {
			if(q == 1) {
				datatemp |= 0x8000;
			}
			else {
				datatemp |= 0x8000;
				datatemp >>= 1;
			}
		} else {
			if(q == 1)
				;
			else
				datatemp >>= 1;
		}
		_delay_cycles(_60us);
	}
	sensor->readData = datatemp;
	return true;
}

void setupTempSensor(DS18B20_t *sensor) {
	oneInit();
	sensor->dataSize = 8;
	sensor->writeData = 0xCC;
	oneWrite(sensor); // skip ROM
	sensor->writeData = 0x4E;
	oneWrite(sensor); // write to scratchpad
	sensor->writeData = 0x7D;
	oneWrite(sensor); // high temp 125C
	sensor->writeData = 0xE6;
	oneWrite(sensor); // low temp -25C
	sensor->writeData = 0x3F;
	oneWrite(sensor); // 10-bit readings
	oneInit();
	sensor->writeData = 0xCC;
	oneWrite(sensor);
	sensor->writeData = 0x48; // copy to EEPROM
	oneWrite(sensor);
	_delay_cycles(_10ms); // 10ms
	oneInit();
}
void checkTempSensor(DS18B20_t *sensor) {
	oneInit();
	sensor->dataSize = 8;
	sensor->writeData = 0xCC;
	oneWrite(sensor);
	sensor->writeData = 0xBE;
	oneWrite(sensor);
	_delay_cycles(_10us); // safety margin (optimizable)
	sensor->dataSize = 16;
	oneRead(sensor); // waste two bytes
	oneRead(sensor); // waste two bytes
	oneRead(sensor); // we want the low byte of this one, high byte should be FF
	if(sensor->readData == 0xFF3F)
		; // all good
	else
		setupTempSensor(sensor); // wrong config data
}

void getTemp(DS18B20_t *sensor) {
	oneInit();
	sensor->dataSize = 8;
	sensor->writeData = 0xCC;
	oneWrite(sensor); // skip ROM
	sensor->writeData = 0x44;
	oneWrite(sensor); // therm conv
	_delay_cycles(_200ms); // wait for 200ms @ 10-bits
	oneInit();
	sensor->writeData = 0xCC;
	oneWrite(sensor);
	sensor->writeData = 0xBE;
	oneWrite(sensor);
	sensor->dataSize = 16;
	oneRead(sensor);
	oneInit();
}
void convThermString(DS18B20_t *sensor) {
	uint16_t temp = sensor->readData;
	float whole;
	whole = (float)(temp >> 4);
	temp = sensor->readData;
	
	// first two aren't needed at 10-bits
//	if(temp & 0x0001)
//		whole += 0.0625;
//	if(temp & 0x0002)
//		whole += 0.125;
	if(temp & 0x0004)
		whole += 0.25;
	if(temp & 0x0008)
		whole += 0.5;
	
	sprintf(sensor->thermData, "%2.2f", whole);
	
}

void doCRC(DS18B20_t *sensor, char CRCval) {
	sensor->CRC = crctable[(sensor->CRC ^ CRCval)];
}

// This header file defines all the registers and values for the
// MAX7219/7221 LED multiplexer. It can do 7-digit decoding (code B),
// or you can control each segment individually by changing the DECMODE
// register. Ensure that both SHUTDWN and DTEST are off for normal operation.

// This file written by MrAureliusR of Frozen Electronics (frozenelectronics.com)
// It's free to re-use and re-distribute, under GPLv2
#include <stdbool.h>
//change this to reflect which pin is used as /CS for MAX7219
#define MAXCS 52

#define DIGIT0 0x0100
#define DIGIT1 0x0200
#define DIGIT2 0x0300
#define DIGIT3 0x0400
#define DIGIT4 0x0500
#define DIGIT5 0x0600
#define DIGIT6 0x0700
#define DIGIT7 0x0800

#define DECMODE   0x0900
#define INTENSE   0x0A00
#define SCANL     0x0B00
#define SHUTDWN   0x0C00
#define DTEST     0x0F00

//decode modes, these need to be bitwise OR'D

#define NODECODE    0x0000
#define CODEB_0     0x0001
#define CODEB_0_3   0x000F
#define CODEB_ALL   0x00FF

//digits for no-decode mode
#define n0	0x007E
#define n1	0x0030
#define n2	0x006D
#define n3	0x0079
#define n4	0x0033
#define n5	0x005B
#define n6	0x005F
#define n7	0x0071
#define n8	0x007F
#define n9	0x007B
//letters
#define lA		0x0077
#define lB		0x001F //lower case (for obvious reasons)
#define lC		0x004E
#define lD		0x003D //lower case again
#define lE		0x004F
#define lF		0x0047
//misc
#define lt		0x000F // lower-case t
#define lP		0x0067 // capital letter P
#define DASH	0x0001 // single segment in middle... self-explanatory
#define DP		0x0080 // decimal point
#define BLANK	0x0000 // all segments off
#define DEG		0x0063 // 'degree' symbol, ie °

//intensity settings
#define DIM       0x0000
#define LEVEL1    0x0001
#define LEVEL2    0x0002
#define LEVEL3    0x0003
#define LEVEL4    0x0004
#define LEVEL5    0x0005
#define LEVEL6    0x0006
#define LEVEL7    0x0007
#define LEVEL8    0x0008
#define LEVEL9    0x0009
#define LEVEL10   0x000A
#define LEVEL11   0x000B
#define LEVEL12   0x000C
#define LEVEL13   0x000D
#define LEVEL14   0x000E
#define FULL      0x000F
//scan limit
#define SCAN_1  0x0000
#define SCAN_2  0x0001
#define SCAN_3  0x0002
#define SCAN_4  0x0003
#define SCAN_5  0x0004
#define SCAN_6  0x0005
#define SCAN_7  0x0006
#define SCAN_8  0x0007
//shutdown
#define ON      0x0000
#define OFF     0x0001
//display test
#define TESTON  0x0001
#define TESTOFF 0x0000

#define ENDLIST 0xFFFF

enum {
	digit0 = 0,
	digit1 = 1,
	digit2 = 2,
	digit3 = 3,
	digit4 = 4,
	digit5 = 5,
	digit6 = 6,
	digit7 = 7,
	intensity = 8,
	decodetype = 9,
	scanl = 10,
	testmode = 11,
	_shutdown = 12
};

typedef struct disp {
	uint16_t registers[14]; // room for all 13 registers and an ENDLIST
	_Bool updated;
	char string[9]; // need space for the null character
} Max7219_t;

// Typically, this should only be called from setDispString,
// but you can call it manually if you want to update all the
// registers in the MAX7219 from the struct.
_Bool setDisplay(Max7219_t *display) {
	int q;
	char byteLow;
	char byteHigh;
	char retLow;
	char retHigh;
	_Bool robust = true;

	for(q = 0; display->registers[q] != ENDLIST; q++) {

		byteLow = display->registers[q];
		byteHigh = (display->registers[q] >> 8);
		//transfer all 16 bits to MAX7219
		SPI.transfer(MAXCS, byteHigh, SPI_CONTINUE);
		SPI.transfer(MAXCS, byteLow);
		//now transfer nops (0x0000) in order to shift out what we just sent
		retHigh = SPI.transfer(MAXCS, 0x00, SPI_CONTINUE);
		retLow = SPI.transfer(MAXCS, 0x00);

		if((retLow == byteLow) && (retHigh == byteHigh))
			;//do nothing
		else
			robust = false;
	}

	return robust;
}

// This function returns the 'display code' for any digit or its ASCII representation.
// Because of this, we need to be VERY careful when passing ASCII values that are
// low numbers (ie, linefeed 0x0A, tab 0x09) because this will not work.
// Essentially, make sure to strip out line feeds BEFORE calling this function.
uint16_t retDispCode(int x) {

	switch(x) {
	case 0:
	case 0x30:
		return n0;
	case 1:
	case 0x31:
		return n1;
	case 2:
	case 0x32:
		return n2;
	case 3:
	case 0x33:
		return n3;
	case 4:
	case 0x34:
		return n4;
	case 5:
	case 0x35:
		return n5;
	case 6:
	case 0x36:
		return n6;
	case 7:
	case 0x37:
		return n7;
	case 8:
	case 0x38:
		return n8;
	case 9:
	case 0x39:
		return n9;
	case 0x0A:
	case 0x41:
		return lA;
	case 0x0B:
	case 0x42:
		return lB;
	case 0x0C:
	case 0x43:
		return lC;
	case 0x0D:
	case 0x44:
		return lD;
	case 0x0E:
	case 0x45:
		return lE;
	case 0x0F:
	case 0x46:
		return lF;
	case 't':
		return lt;
	case 'P':
		return lP;
	case '-':
		return DASH;
	case '.':
		return DP;
	case ' ':
		return BLANK;
	case '*':			// since the degree symbol is non-standard, use *
		return DEG;

	default:			// just in case
		return BLANK;
	}
}
uint16_t retPosCode(int x) {
	switch(x) {
	case 0:
		return DIGIT0;
	case 1:
		return DIGIT1;
	case 2:
		return DIGIT2;
	case 3:
		return DIGIT3;
	case 4:
		return DIGIT4;
	case 5:
		return DIGIT5;
	case 6:
		return DIGIT6;
	case 7:
		return DIGIT7;
	default:
		return 0;
	}
}

_Bool setDispString(Max7219_t *display) {
	char s;
	int z;
	int d = 0;
	for(z = 0; z < 8; z++) {
		s = (display->string[z]);
		
		if(s == '\0')
			break;
		else {
			if(display->string[z+1] == '.') {
				display->registers[d] = (retPosCode(d) | retDispCode((int)s) | DP);
				d++;
				z++;
			}
			else {
				display->registers[d] = (retPosCode(d) | retDispCode((int)s));
				d++;
			}
		}
	}
	return (setDisplay(display));
}




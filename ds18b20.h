// This header file contains all the functions for working with a
// Dallas DS18B20 1-wire temperature sensor. At this point I will
// be writing the functions myself, however if it doesn't operate
// as well as I intend I might use the 1-Wire library from Arduino.
#include <stdbool.h>

typedef struct temperatureSense {
	int16_t data;
	_Bool updated;
	char thermData[14];
} DS18B20_t;



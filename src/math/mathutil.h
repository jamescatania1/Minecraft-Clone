#ifndef _mathutil_h
#define _mathutil_h

#include <math.h>

float fmodulo(float f, float d) {
	while (f < 0) f += d;
	return fmodf(f, d);
}

#endif
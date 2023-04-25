#ifndef _noise_h
#define _noise_h

typedef struct PerlinNoise {
	int key;
	int p[512];
} *PerlinNoise;

extern PerlinNoise new_PerlinNoise(int key);

extern void PerlinNoise_free(PerlinNoise noise);

//Returns noise value within 0 and 1
extern double perlinNoise(PerlinNoise noise, double x, double y);


typedef struct OctaveNoise {
	int key;
	PerlinNoise* octaves;
	int octaveCt;

	//common value: 0.5
	double ampOctaveMultiplier;

	//common value: 2.0
	double freqOctaveMultiplier;
} *OctaveNoise;

//Common value for ampOctaveMultiplier : 0.5; Common value for freqOctaveMultiplier : 2.0
OctaveNoise OctaveNoise_set(int key, int octaveCt, double ampOctaveMultiplier, double freqOctaveMultiplier);

void OctaveNoise_free(OctaveNoise noise);

double octaveNoise(double x, double y);

#endif
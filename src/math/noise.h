#ifndef _noise_h
#define _noise_h

typedef struct PerlinNoise {
	char key[256];
	int p[512];
	double xOffset, yOffset;
} *PerlinNoise;

extern PerlinNoise new_PerlinNoise(char key[256], double xOffset, double yOffset);

extern void PerlinNoise_free(PerlinNoise noise);

//Returns noise value within 0 and 1
extern double perlinNoise(PerlinNoise noise, double x, double y);

typedef struct OctaveNoise {
	//char* key;
	PerlinNoise* octaves;
	int octaveCt;

	double amplitude, frequency, offset;

	//common value: 0.5
	double ampOctaveMultiplier;

	//common value: 2.0
	double freqOctaveMultiplier;
} *OctaveNoise;

//Common value for ampOctaveMultiplier : 0.5; Common value for freqOctaveMultiplier : 2.0
extern OctaveNoise new_OctaveNoise(double amplitude, double frequency, double offset, int octaveCt, double ampOctaveMultiplier, double freqOctaveMultiplier);

//should be set at beginning of world creation/load 
void OctaveNoise_setseed(unsigned int seed);

unsigned int OctaveNoise_getseed();

extern void OctaveNoise_free(OctaveNoise noise);

extern double octaveNoise(OctaveNoise noise, double x, double y);

#endif
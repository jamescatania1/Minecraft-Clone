#ifndef _noise_h
#define _noise_h

typedef struct PerlinNoise {
	char key[256];
	int p[512];
} *PerlinNoise;

extern PerlinNoise new_PerlinNoise(char key[256]);

extern void PerlinNoise_free(PerlinNoise noise);

//Returns noise value within 0 and 1
extern double perlinNoise(PerlinNoise noise, double x, double y);


typedef struct OctaveNoise {
	unsigned int seed;
	char* key;
	PerlinNoise* octaves;
	int octaveCt;

	//common value: 0.5
	double ampOctaveMultiplier;

	//common value: 2.0
	double freqOctaveMultiplier;
} *OctaveNoise;

//Common value for ampOctaveMultiplier : 0.5; Common value for freqOctaveMultiplier : 2.0
extern OctaveNoise OctaveNoise_set(unsigned int seed, int octaveCt, double ampOctaveMultiplier, double freqOctaveMultiplier);

extern void OctaveNoise_free(OctaveNoise noise);

extern double octaveNoise(double x, double y);

#endif
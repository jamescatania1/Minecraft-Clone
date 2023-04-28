#pragma warning(disable: 6011)
#pragma warning(disable: 6386)

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "noise.h"

double lerp(double t, double a, double b);
double fade(double t);
double grad(int hash, double x, double y);

OctaveNoise activeNoise;

static int permutation[] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

PerlinNoise new_PerlinNoise(char key[256]) {
	PerlinNoise this = (PerlinNoise)malloc(sizeof(struct PerlinNoise));
	for (int i = 0; i < 256; i++) {
		this->key[i] = key[i];
		this->p[i] = permutation[(int)key[i] + 128];
		this->p[256 + i] = this->p[i];
	}
	return this;
}

void PerlinNoise_free(PerlinNoise noise) {
	free(noise);
}

double perlinNoise(PerlinNoise noise, double x, double y) {
	int xpos = (int)floor(x) % 256;
	if (xpos < 0) xpos += 256;
	int ypos = (int)floor(y) % 256;
	if (ypos < 0) ypos += 256;
	double _x = fmod(x, 1.0);
	if (_x < 0.0f) _x += 1.0f;
	double _y = fmod(y, 1.0);
	if (_y < 0.0f) _y += 1.0f;

	double xfade = fade(_x); //fade curves
	double yfade = fade(_y);

	int hash_a = noise->p[xpos] + ypos;
	int hash_b = noise->p[xpos + 1] + ypos;
	int aa = noise->p[hash_a];
	int ba = noise->p[hash_b];
	int ab = noise->p[hash_a + 1];
	int bb = noise->p[hash_b + 1];

	int hash_11 = noise->p[aa];
	int hash_01 = noise->p[ba];
	int hash_10 = noise->p[ab];
	int hash_00 = noise->p[bb];

	double result = lerp(yfade,
		lerp(xfade, grad(hash_11, _x, _y), grad(hash_01, _x - 1.0, _y)),
		lerp(xfade, grad(hash_10, _x, _y - 1.0), grad(hash_00, _x - 1.0, _y - 1.0)));
	return 0.5 + result;
}

double lerp(double t, double a, double b) {
	return a + t * (b - a); 
}

double fade(double t) {
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double grad(int hash, double x, double y) {
	return ((hash & 1) ? x : -x) + ((hash & 2) ? y : -y);
}


// Octave noise

char* OctaveNoise_key_generate(unsigned int seed) {
	srand(seed);
	int key[256];
	char* result = (char*)malloc(256 * sizeof(char));
	for (int i = 0; i < 256; i++) key[i] = -1;
	for (int i = 0; i < 256; i++) {
		while (key[i] == -1) {
			int val = rand() % 256;
			int foundVal = 0;
			for (int j = 0; j < 256; j++) {
				if (key[j] == val) {
					foundVal = 1;
					break;
				}
			}
			if (!foundVal) key[i] = val;
		}
		result[i] = (char)key[i];
	}
	//printf("Seed: %u\n", seed);
	//printf("%s\n", result);
	return result;
}

//updates active octave noise and returns the object
OctaveNoise OctaveNoise_set(unsigned int seed, int octaveCt, double ampOctaveMultiplier, double freqOctaveMultiplier) {
	OctaveNoise this = (OctaveNoise)malloc(sizeof(struct OctaveNoise));
	if (!seed) { //generate random seed
		srand(time(NULL));
		unsigned int genseed = 0;
		for (int i = 0; i < 9; i++) {
			genseed *= 10;
			genseed += rand() % 10;
		}
		genseed *= 10;
		genseed += rand() % 4;
		this->seed = genseed;
	}
	else this->seed = seed;

	this->key = OctaveNoise_key_generate(this->seed);
	this->octaveCt = octaveCt;
	this->ampOctaveMultiplier = ampOctaveMultiplier;
	this->freqOctaveMultiplier = freqOctaveMultiplier;
	this->octaves = (PerlinNoise*)malloc(octaveCt * sizeof(PerlinNoise));
	for (int i = 0; i < octaveCt; i++) {
		this->octaves[i] = new_PerlinNoise(this->key);
	}
	activeNoise = this;
	return this;
}

void OctaveNoise_free(OctaveNoise noise) {
	for (int i = 0; i < noise->octaveCt; i++) {
		PerlinNoise_free(noise->octaves[i]);
	}
	free(noise->octaves);
	free(noise->key);
	free(noise);
}

double octaveNoise(double x, double y) {
	double result = 0.0;
	double amp = 1.0;
	double freq = 1.0;
	double totalAmp = 0.0;
	for (int i = 0; i < activeNoise->octaveCt; i++) {
		result += amp * perlinNoise(activeNoise->octaves[i], x * freq, y * freq);
		totalAmp += amp;
		amp *= activeNoise->ampOctaveMultiplier;
		freq *= activeNoise->freqOctaveMultiplier;
	}
	return result / totalAmp;
}
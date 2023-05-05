#ifndef _chunk_h
#define _chunk_h

#include "renderComponent.h"
#include "world.h"

#define BLOCK_TYPE int 

typedef struct Chunk {
	int cull;
	int posX;
	int posZ;
	int setMesh;
	char data[16][16][256];
	RenderComponent renderer;
} *Chunk;

// Generate chunk at chunk location x, z
extern Chunk Chunk_generate(World world, int xPos, int zPos);

extern void Chunk_updatemesh(Chunk chunk, Chunk neighboringData[4]);

extern void Chunk_free(Chunk chunk);

extern int Chunk_hash(Chunk c);

extern int Chunk_hash_pos(int x, int z);

#endif
#ifndef _world_h
#define _world_h

#include "camera.h"
#include "structures/list.h"
#include "structures/linkedlist.h"
#include "structures/hashmap.h"
#include "structures/hashset.h"
#include "math/noise.h"
#include "shader.h"
#include "skybox.h"

typedef struct BiomeInfo {
	OctaveNoise heightmap;
} *BiomeInfo;

extern BiomeInfo new_BiomeInfo(OctaveNoise heightmap);

extern void BiomeInfo_free(BiomeInfo biome);

typedef struct World {
	unsigned int cameraUBO;
	Skybox skybox;

	//biome info
	BiomeInfo biomeInfo[3];

	OctaveNoise oceanMap;

	//main list of all currently loaded chunks
	List chunks;

	//main map of Int2 -> Chunk of all chunks that are currently loaded
	HashMap chunkmap;

	//set of Int2(int) of chunks that request generation/load
	HashSet scheduledLoadChunks;

	//set of Int2(int) for chunks that request a mesh update
	HashSet scheduledMeshUpdateChunks;

	//queue of Int2's of chunk positions that request generation/load
	LinkedList chunkLoadQueue;

	//queue of Int2's of chunk positions that request a mesh update
	LinkedList chunkMeshUpdateQueue;

	//shader shared by all chunks
	Shader chunkShader;
} *World;

extern World new_World();

extern void World_free(World world);

extern void World_update(World world);
 
extern void World_fixedupdate(World world);

extern void World_draw(World world);

#endif
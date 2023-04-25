#ifndef _world_h
#define _world_h

#include "camera.h"
#include "structures/list.h"
#include "structures/linkedlist.h"
#include "structures/hashmap.h"
#include "math/noise.h"
#include "shader.h"
#include "skybox.h"

typedef struct World {
	unsigned int cameraUBO;
	OctaveNoise noise;
	Skybox skybox;

	//main list of all currently loaded chunks
	List chunks;

	//main map of Int2 -> Chunk of all chunks that are currently loaded
	HashMap chunkmap;

	//map of Int2 -> Int2 of chunks that request generation/load
	HashMap scheduledLoadChunks;

	//map of Int2 -> Chunk for chunks that request a mesh update
	HashMap scheduledMeshUpdateChunks;

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
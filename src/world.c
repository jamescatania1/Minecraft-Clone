//the story of galois, ..., he was in a dual with another man because of a girl, ..., so, fortunately or unfortunately, depending on how you view, we don't have more galois theory
//-arda demirhan, 4/24/22
#pragma warning(disable: 6011)
#pragma warning(disable: 6286)

#include <glad/glad.h>
#include <glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "world.h"
#include "main.h"
#include "input.h"
#include "camera.h"
#include "renderComponent.h"
#include "structures/list.h"
#include "chunk.h"
#include "blockdef.h"
#include "atlas.h"
#include "math/noise.h"

//temporary
#define SEED_INPUT 5

#define LOCK_CURSOR_DEFAULT 1
#define FRUSTUM_CULLING 1

int cursorLocked;
int prevCamX;
int prevCamZ;

BiomeInfo new_BiomeInfo(OctaveNoise heightmap) {
	BiomeInfo this = (BiomeInfo)malloc(sizeof(struct BiomeInfo));
	if (!this) return NULL;

	this->heightmap = heightmap;
	return this;
}

void BiomeInfo_free(BiomeInfo biome) {
	OctaveNoise_free(biome->heightmap);
	free(biome);
}

World new_World() {
	World this = (World)malloc(sizeof(struct World));
	if (!this) return NULL;

	//lock cursor
	cursorLocked = LOCK_CURSOR_DEFAULT;
	if (cursorLocked) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//create global buffer object for camera data
	glGenBuffers(1, &this->cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, this->cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, 136, NULL, GL_DYNAMIC_DRAW); //2 * 4 * 16 bytes + 4 + 4
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//initialize texture atlas
	TextureAtlas_init();

	//initialize noise
	OctaveNoise_setseed(SEED_INPUT);
	
	this->oceanMap = new_OctaveNoise(1.0, 0.0015, 0.0, 5, 0.3, 2.2);

	//ocean biome
	this->biomeInfo[BIOME_OCEAN] = new_BiomeInfo(new_OctaveNoise(23.0, 0.015, 15.0, 4, 0.7, 1.8));

	//plains biome
	this->biomeInfo[BIOME_PLAINS] = new_BiomeInfo(new_OctaveNoise(50.0, 0.0075, 50.0, 6, 0.5, 2.0));

	//desert biome
	this->biomeInfo[BIOME_DESERT] = new_BiomeInfo(new_OctaveNoise(30.0, 0.0045, 45.0, 6, 0.5, 2.0));

	//load chunkShader
	this->chunkShader = new_Shader("shaders\\default_vert.vshader", "shaders\\default_frag.fshader");
	//set matrix transform uniform location index to "0"
	Shader_uniformLocations[0] = glGetUniformLocation(this->chunkShader->id, "transform");

	//initialize skybox
	this->skybox = new_Skybox();
	Camera_UpdateMatrix();
	//set global buffer object data for camera near/far
	glBindBuffer(GL_UNIFORM_BUFFER, this->cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 128, 4, &camera->zNear);
	glBufferSubData(GL_UNIFORM_BUFFER, 132, 4, &camera->zFar);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//initialize chunks list/map
	this->chunks = new_List();
	this->chunkmap = new_HashMap(25);
	this->scheduledLoadChunks = new_HashSet(25);
	//this->scheduledMeshUpdateChunks = new_HashSet(25);

	//initialize chunk load queue and mesh update queue
	this->chunkLoadQueue = new_LinkedList();
	//this->chunkMeshUpdateQueue = new_LinkedList();

	prevCamX = prevCamZ = -2;

	return this;
}

void World_free(World world) {
	//LinkedList_freeAll(world->chunkMeshUpdateQueue, Int2_free);
	LinkedList_freeAll(world->chunkLoadQueue, Int2_free);
	//HashSet_free(world->scheduledMeshUpdateChunks);
	HashSet_free(world->scheduledLoadChunks);
	List_freeAll(world->chunks, Chunk_free);
	HashMap_free(world->chunkmap);
	Shader_free(world->chunkShader);
	Skybox_free(world->skybox);
	Camera_free();
	TextureAtlas_free();
	for (int i = 0; i < 3; i++) BiomeInfo_free(world->biomeInfo[i]);
	OctaveNoise_free(world->oceanMap);
	free(world);
}

inline Chunk ChunkGetOrLoad(World world, int x, int z);
void QSList(List in, float* compare, int lowIndx, int hiIndx);
int QSPartition(List in, float* compare, int lowIndx, int hiIndx);

void World_update(World world) {
	Camera_UpdateMatrix();

	//set global buffer object data for camera
	glBindBuffer(GL_UNIFORM_BUFFER, world->cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &camera->viewMatrix->data[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, &camera->projMatrix->data[0][0]);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, world->cameraUBO);

	if (getKey(KEY_SPACE)->pressed) {
		int numTris = 0;
		int viewingchunks = 0;
		for (int i = 0; i < world->chunks->capacity; i++) {
			Chunk c = (Chunk)List_get(world->chunks, i);
			if (!c) continue;
			if (!c->cull) viewingchunks++;
			if (!c->cull) numTris += c->renderer->indexCount / 3;
		}
		printf("== World Info ==\n%u seed\n%d render distance\n%d chunks loaded\n%d chunks loaded (not culled)\n%d triangles loaded (not culled)\n================\n",
			OctaveNoise_getseed(), RENDER_DISTANCE, world->chunks->count, viewingchunks, numTris);
	}
	if (getKey(KEY_G)->pressed) {
		printf("Camera %f %f\n", camera->position->x, camera->position->z);
	}

	if (getKey(KEY_ESCAPE)->pressed) {
		cursorLocked = !cursorLocked;
		if (cursorLocked) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPos(window, windowX / 2, windowY / 2);
			updateInputManager();
		}
	}
	
	int camX = (int)floorf(camera->position->x / 16.0f);
	int camZ = (int)floorf(-camera->position->z / 16.0f);

	//update scheduled chunk meshes
	if (world->chunkLoadQueue->count > 0) {
		LinkedListNode chunkPosNode = LinkedList_polllast(world->chunkLoadQueue);
		Int2 chunkpos = (Int2)chunkPosNode->data;
		int cx = chunkpos->x; int cz = chunkpos->y;
		Chunk c = ChunkGetOrLoad(world, cx, cz);
		//HashSet_remove(world->scheduledLoadChunks, Chunk_hash(c));
		free(chunkPosNode);
		free(chunkpos);
		
		Chunk c_l = ChunkGetOrLoad(world, cx - 1, cz);
		Chunk c_r = ChunkGetOrLoad(world, cx + 1, cz);
		Chunk c_b = ChunkGetOrLoad(world, cx, cz - 1);
		Chunk c_f = ChunkGetOrLoad(world, cx, cz + 1);
		Chunk neighbors[4] = { c_l, c_r, c_b, c_f };
		Chunk_updatemesh(c, neighbors);
	}

	//camera moved between chunks
	if (camX != prevCamX || camZ != prevCamZ) {
		for (int i = 1; i <= RENDER_DISTANCE + 1; i++) {
			for (int x = -i + camX; x <= i + camX; x++) { //schedule load of new chunks
				for (int z = -i + camZ; z <= i + camZ; z++) {
					double camDistance = sqrt((double)((x - camX) * (x - camX) + (z - camZ) * (z - camZ)));
					if (camDistance > (double)(RENDER_DISTANCE + 1) + 0.5) continue;

					if(HashSet_contains(world->scheduledLoadChunks, Chunk_hash_pos(x, z))) continue;
					HashSet_insert(world->scheduledLoadChunks, Chunk_hash_pos(x, z));
					LinkedList_prepend(world->chunkLoadQueue, new_Int2(x, z));
				}
			}
		}
	}

	for (int i = 0; i < world->chunks->count; i++) { //unload chunks out of render distance
		Chunk c = (Chunk)List_get(world->chunks, i);
		if (!c) continue;
		double camDistance = sqrt((double)((c->posX - camX) * (c->posX - camX) + (c->posZ - camZ) * (c->posZ - camZ)));
		if (camDistance > (double)(RENDER_DISTANCE + 1) + 0.5) {
			List_remove(world->chunks, i);
			HashMap_remove(world->chunkmap, Chunk_hash(c));
			HashSet_remove(world->scheduledLoadChunks, Chunk_hash(c));
			Chunk_free(c);
		}
	}

	prevCamX = camX;
	prevCamZ = camZ;

	//frustum culling
	Vec3 camForward = new_Vec3(camera->viewMatrix->data[0][2], camera->viewMatrix->data[1][2], camera->viewMatrix->data[2][2]);
	Vec3_normalize(camForward);
	for (int i = 0; i < world->chunks->count; i++) {
		Chunk c = (Chunk)List_get(world->chunks, i);
		if (!c) continue;
		Vec3 cvec = new_Vec3((float)c->posX * 16.0f - camera->position->x, 60.0f - camera->position->y, (float)c->posZ * 16.0f + camera->position->z);
		float dist = Vec3_magnitude(cvec);
		Vec3_normalize(cvec);
		c->cull = Vec3_dot(cvec, camForward) > 0.15f && dist > 48.0f;
		free(cvec);
	}
	free(camForward);

	//sort chunks
	float* compare = (float*)malloc(world->chunks->count * sizeof(float));
	for (int i = 0; i < world->chunks->count; i++) {
		Chunk c = (Chunk)List_get(world->chunks, i);
		float c_x = c->posX * 16.0 + 8.0;
		float c_z = c->posZ * 16.0 + 8.0;
		compare[i] = sqrtf((c_x - camera->position->x) * (c_x - camera->position->x) + (c_z + camera->position->z) * (c_z + camera->position->z));
	}
	QSList(world->chunks, compare, 0, world->chunks->count - 1);
	free(compare);
}

inline Chunk ChunkGetOrLoad(World world, int x, int z) {
	Chunk c = (Chunk)HashMap_get(world->chunkmap, Chunk_hash_pos(x, z));
	if (!c) {
		c = Chunk_generate(world, x, z);
		List_add(world->chunks, c);
		HashMap_insert_at(world->chunkmap, Chunk_hash(c), c);
	}
	return c;
}

void QSList(List in, float* compare, int lowIndx, int hiIndx) {
	//quicksort algorithm based on distance val per chunk
	if (lowIndx >= hiIndx) return; //base case
	int lowIndxEnd = QSPartition(in, compare, lowIndx, hiIndx);
	QSList(in, compare, lowIndx, lowIndxEnd);
	QSList(in, compare, lowIndxEnd + 1, hiIndx);
}
int QSPartition(List in, float* compare, int lowIndx, int hiIndx) {
	int midindx = lowIndx + (hiIndx - lowIndx) / 2;
	float pivot = compare[midindx];
	while (1) {
		while (compare[lowIndx] > pivot) lowIndx++;
		while (compare[hiIndx] < pivot) hiIndx--;
		if (lowIndx >= hiIndx) break;
		else {
			void* tmp = List_get(in, lowIndx);
			List_set(in, List_get(in, hiIndx), lowIndx);
			List_set(in, tmp, hiIndx);
			float ctmp = compare[lowIndx];
			compare[lowIndx] = compare[hiIndx];
			compare[hiIndx] = ctmp;
			lowIndx++;
			hiIndx--;
		}
	}
	return hiIndx;
}
void World_fixedupdate(World world) {

}

void World_draw(World world) {
	//render skybox
	glDepthFunc(GL_LEQUAL);
	Shader_use(world->skybox->renderer->shader);
	Shader_setMat4x4(world->skybox->renderer->shader, 1, camera->rotProjMatrix);
	glBindVertexArray(world->skybox->renderer->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, world->skybox->renderer->texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureAtlas_currentTexture());
	Shader_use(world->chunkShader);
	for (int i = 0; i < world->chunks->capacity; i++) {
		Chunk chunk = (Chunk)List_get(world->chunks, i);
		if (!chunk || !chunk->setMesh) continue;
		if (chunk->cull && FRUSTUM_CULLING) continue;
		RenderComponent renderer = chunk->renderer;
		if (!renderer) continue;
		if (!renderer->transformStatic) RenderComponent_updatetransformmatrix(renderer);

		Shader_setMat4x4(world->chunkShader, 0, &renderer->transform->matrix->data[0][0]);
		glBindVertexArray(renderer->VAO);
		glDrawElements(GL_TRIANGLES, renderer->indexCount, GL_UNSIGNED_INT, 0);
	}
}
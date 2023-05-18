//the story of galois, ..., he was in a dual with another man because of a girl, ..., so, fortunately or unfortunately, depending on how you view, we don't have more galois theory
//-arda demirhan, 4/24/22
#pragma warning(disable: 6011)
#pragma warning(disable: 6286)

#include <glad/glad.h>
#include <glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "stringutil.h"
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
#include "math/linearalgebra.h"

//temporary
#define SEED_INPUT 0

#define LOCK_CURSOR_DEFAULT 1
#define FRUSTUM_CULLING 1

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

int cursorLocked;
int prevCamX;
int prevCamZ;
int prevCamBlockX;
int prevCamBlockZ;

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
	glBufferData(GL_UNIFORM_BUFFER, 136 + 64 * SHADOW_CASCADES, NULL, GL_DYNAMIC_DRAW); //2 * 4 * 16 bytes + 4 + 4
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

	this->shadowmapShader = new_Shader("shaders\\shadowmap.vert", "shaders\\shadowmap.geom", "shaders\\shadowmap.frag");
	Shader_uniformLocations[UNIFORM_SHADOWMAP_VERT_TRANSFORM] = glGetUniformLocation(this->shadowmapShader->id, "transform");

	//load chunkShader
	this->chunkShader = new_Shader("shaders\\default.vert", NULL, "shaders\\default.frag");
	//set matrix transform uniform location index
	Shader_uniformLocations[UNIFORM_DEFAULT_VERT_TRANSFORM] = glGetUniformLocation(this->chunkShader->id, "transform");
	Shader_setInt(this->chunkShader, "atlas", 0);
	Shader_setInt(this->chunkShader, "shadowMap", 1);
	for (int i = 0; i < SHADOW_CASCADES; i++) {
		char buffer[10];
		snprintf(buffer, 10, "%d", i);
		char* nameA = concat(buffer, "]");
		char* nameFull = concat("cascadePlaneDistances[", nameA);
		Shader_setFloat(this->chunkShader, nameFull, SHADOW_CASCADE_DISTANCES[i]);
		Shader_setFloat(this->shadowmapShader, nameFull, SHADOW_CASCADE_DISTANCES[i]);
		free(nameA); free(nameFull);
	}

	//initialize shadowmap depth buffer/fbo
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glGenFramebuffers(1, &this->shadowmapFBO);
	glGenTextures(1, &this->depthBuffer);
	glBindTexture(GL_TEXTURE_2D_ARRAY, this->depthBuffer);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, SHADOW_CASCADES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, this->shadowmapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->depthBuffer, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("FBO did not load\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowX, windowY);

	this->sunViewProjMatrix = (Mat4x4*)malloc(SHADOW_CASCADES * sizeof(Mat4x4));
	for (int i = 0; i < SHADOW_CASCADES; i++) {
		this->sunViewProjMatrix[i] = NULL;
	}

	this->debugQuadShader = new_Shader("shaders\\debug_screenquad.vert", NULL, "shaders\\debug_screenquad.frag");
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadIndices[] = {
		0, 1, 2, 0, 2, 3
	};
	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	unsigned int quadEBO;
	glGenBuffers(1, &quadEBO);
	this->debugQuadVAO = 0;
	glGenVertexArrays(1, &this->debugQuadVAO);
	glBindVertexArray(this->debugQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, quadVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, quadIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); //texture coords
	glEnableVertexAttribArray(1);

	//initialize skybox
	this->skybox = new_Skybox();
	Camera_UpdateMatrix();
	//set global buffer object data for camera near/far
	glBindBuffer(GL_UNIFORM_BUFFER, this->cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 128 + 64 * SHADOW_CASCADES, 4, &camera->zNear);
	glBufferSubData(GL_UNIFORM_BUFFER, 128 + 64 * SHADOW_CASCADES + 4, 4, &camera->zFar);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//initialize chunks list/map
	this->chunks = new_List();
	this->chunkmap = new_HashMap(25);
	this->scheduledLoadChunks = new_HashSet(25);

	//initialize chunk load queue
	this->chunkLoadQueue = new_LinkedList();
	this->sortScheduled = 1;

	prevCamX = prevCamZ = -2;
	prevCamBlockX = prevCamBlockZ = (int)floorf(camera->position->x) - 1;

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
	for (int i = 0; i < SHADOW_CASCADES; i++) {
		Mat4x4_free(world->sunViewProjMatrix[i]);
	}
	free(world->sunViewProjMatrix);
	Shader_free(world->debugQuadShader);
	Shader_free(world->shadowmapShader);
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
	int camX = (int)floorf(camera->position->x / 16.0f);
	int camZ = (int)floorf(camera->position->z / 16.0f);
	int camBlockX = (int)floorf(camera->position->x);
	int camBlockZ = (int)floorf(camera->position->z);

	//update main camera matrix
	Camera_UpdateMatrix();


	//set global buffer object data for camera
	glBindBuffer(GL_UNIFORM_BUFFER, world->cameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &camera->viewMatrix->data[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, &camera->projMatrix->data[0][0]);
	if (camBlockX != prevCamBlockZ || camBlockZ != prevCamBlockZ) {
		for (int i = 0; i < SHADOW_CASCADES; i++) {
			Mat4x4_free(world->sunViewProjMatrix[i]);
			world->sunViewProjMatrix[i] = Mat4x4_ViewProjOrthographic(camera->position->x + 20.0f, 200.0f, camera->position->z - 48.0f, 70.0f, 30.0f, 0.0f, 200.0f, 1.3f * SHADOW_CASCADE_DISTANCES[i]);
			glBufferSubData(GL_UNIFORM_BUFFER, 128 + 64 * i, 64, &world->sunViewProjMatrix[i]->data[0][0]);
		}
	}
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
		printf("==Camera==\nposition: (%f, %f, %f)\nforward: (%f, %f, %f)\nright: (%f, %f, %f)\nup: (%f, %f, %f)\n===============\n", 
			camera->position->x, camera->position->y, camera->position->z,
			camera->forward->x, camera->forward->y, camera->forward->z,
			camera->right->x, camera->right->y, camera->right->z,
			camera->up->x, camera->up->y, camera->up->z);
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
		Shader_use(world->chunkShader);
		world->sortScheduled = 1;
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

	//frustum culling
	for (int i = 0; i < world->chunks->count; i++) {
		//for debugging: hold space to pause frustum culling
		if (getKey(KEY_SPACE)->down) break;
		
		Chunk c = (Chunk)List_get(world->chunks, i);
		if (!c) continue;

		c->cull = 0;

		float cp_x = c->posX * 16.0f + 8.0f - camera->position->x;
		float cp_z = c->posZ * 16.0f + 8.0f - camera->position->z;

		for (int j = 0; j < 6; j++) {
			Camera_FrustumPlane plane = camera->frustum[j];

			if (cp_x * plane->normal->x - camera->position->y * plane->normal->y + cp_z * plane->normal->z < plane->distance
				&& cp_x * plane->normal->x + (c->maxHeight - camera->position->y) * plane->normal->y + cp_z * plane->normal->z < plane->distance) {
				c->cull = 1;
				break;
			}
		}
	}
	for (int i = camX - 2; i <= camX + 2; i++) { //keep 25 closest chunks to camera from being culled
		for (int j = camZ - 2; j <= camZ + 2; j++) {
			Chunk c = HashMap_get(world->chunkmap, Chunk_hash_pos(i, j));
			if (!c) continue;
			c->cull = 0;
		}
	}
	
	//sort chunks
	if (world->sortScheduled || camBlockX != prevCamBlockX || camBlockZ != prevCamBlockZ) {
		world->sortScheduled = 0;
		float* compare = (float*)malloc(world->chunks->count * sizeof(float));
		for (int i = 0; i < world->chunks->count; i++) {
			Chunk c = (Chunk)List_get(world->chunks, i);
			float c_x = c->posX * 16.0 + 8.0;
			float c_z = c->posZ * 16.0 + 8.0;
			compare[i] = sqrtf((c_x - camera->position->x) * (c_x - camera->position->x) + (c_z - camera->position->z) * (c_z - camera->position->z));
		}
		QSList(world->chunks, compare, 0, world->chunks->count - 1);
		free(compare);
	}

	prevCamX = camX;
	prevCamZ = camZ;
	prevCamBlockX = camBlockX;
	prevCamBlockZ = camBlockZ;
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

void World_renderscene(World world, Shader shader, int transformUniformLocation);
void World_draw(World world) {
	//render to depth buffer from sun perspective
	Shader_use(world->shadowmapShader);
	Shader_setBool(world->shadowmapShader, "minCascade", 1);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, world->shadowmapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	World_renderscene(world, world->shadowmapShader, UNIFORM_SHADOWMAP_VERT_TRANSFORM);

	//render rest of scene
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	setViewportSize(windowX, windowY);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render skybox
	glDepthFunc(GL_LEQUAL);
	Shader_use(world->skybox->renderer->shader);
	Shader_setMat4x4(world->skybox->renderer->shader, UNIFORM_SKYBOX_MATRIX, camera->rotProjMatrix);
	glBindVertexArray(world->skybox->renderer->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, world->skybox->renderer->texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	
	//render world
	Shader_use(world->chunkShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureAtlas_currentTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, world->depthBuffer);
	World_renderscene(world, world->chunkShader, UNIFORM_DEFAULT_VERT_TRANSFORM);

	/*
	//view depth buffer (debugging)
	Shader_use(world->debugQuadShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, world->depthBuffer);
	glBindVertexArray(world->debugQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/
}

void World_renderscene(World world, Shader shader, int transformUniformLocation) {
	for (int i = 0; i < world->chunks->capacity; i++) {
		Chunk chunk = (Chunk)List_get(world->chunks, i);
		if (!chunk || !chunk->setMesh) continue;
		if (chunk->cull && FRUSTUM_CULLING) continue;
		RenderComponent renderer = chunk->renderer;
		if (!renderer) continue;
		if (!renderer->transformStatic) {
			RenderComponent_updatetransformmatrix(renderer);
		}
		if(transformUniformLocation != -1) 
			Shader_setMat4x4(shader, transformUniformLocation, &renderer->transform->matrix->data[0][0]);

		glBindVertexArray(renderer->VAO);
		glDrawElements(GL_TRIANGLES, renderer->indexCount, GL_UNSIGNED_INT, 0);
	}
}
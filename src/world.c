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
#include "atlas.h"
#include "math/noise.h"

#define LOCK_CURSOR_DEFAULT 1
#define FRUSTUM_CULLING 1

int cursorLocked;
int prevCamX;
int prevCamZ;

World new_World() {
	World this = (World)malloc(sizeof(struct World));

	//lock cursor
	cursorLocked = LOCK_CURSOR_DEFAULT;
	if (cursorLocked) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//initialize camera

	//create global buffer object for camera data
	glGenBuffers(1, &this->cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, this->cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, 136, NULL, GL_DYNAMIC_DRAW); //2 * 4 * 16 bytes + 4 + 4
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//initialize texture atlas
	TextureAtlas_init();

	//initialize noise
	this->noise = OctaveNoise_set(0, 5, 0.5, 2.0);

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
	this->scheduledLoadChunks = new_HashMap(25);
	this->scheduledMeshUpdateChunks = new_HashMap(25);

	//initialize chunk load queue and mesh update queue
	this->chunkLoadQueue = new_LinkedList();
	this->chunkMeshUpdateQueue = new_LinkedList();

	prevCamX = prevCamZ = -2;

	return this;
}

void World_free(World world) {
	LinkedList_freeAll(world->chunkMeshUpdateQueue, Int2_free);
	LinkedList_freeAll(world->chunkLoadQueue, Int2_free);
	HashMap_free(world->scheduledMeshUpdateChunks);
	HashMap_free_all(world->scheduledLoadChunks, Int2_free);
	List_freeAll(world->chunks, Chunk_free);
	HashMap_free(world->chunkmap);
	Shader_free(world->chunkShader);
	Skybox_free(world->skybox);
	Camera_free();
	OctaveNoise_free(world->noise);
	TextureAtlas_free();
	free(world);
}

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
			numTris += c->renderer->indexCount / 3;
		}
		printf("== World Info ==\n%d render distance\n%d chunks loaded\n%d chunks loaded (not culled)\n%d triangles loaded\n================\n", 
			RENDER_DISTANCE, world->chunks->activeCount, viewingchunks, numTris);
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
	
	int camX = (int)(camera->position->x / 16.0f);
	int camZ = (int)(-camera->position->z / 16.0f);

	//evaluates true when moved between chunks
	if (camX != prevCamX || camZ != prevCamZ) {
	}

	//load scheduled chunks
	if (world->chunkLoadQueue->count > 0) {
		LinkedListNode chunkPosNode = LinkedList_polllast(world->chunkLoadQueue);
		Int2 chunkpos = (Int2)chunkPosNode->data;
		Chunk c = Chunk_generate(chunkpos->x, chunkpos->y);
		List_add(world->chunks, c);
		HashMap_insert_at(world->chunkmap, Chunk_hash(c), c);
		HashMap_remove_free(world->scheduledLoadChunks, Int2_free, Chunk_hash(c));
		free(chunkPosNode);
		free(chunkpos);
	}

	//update scheduled chunk meshes
	if (world->chunkMeshUpdateQueue->count > 0) {
		LinkedListNode chunkPosNode = LinkedList_polllast(world->chunkMeshUpdateQueue);
		Int2 chunkpos = (Int2)chunkPosNode->data;
		Chunk c = (Chunk)HashMap_get(world->scheduledMeshUpdateChunks, Chunk_hash_pos(chunkpos->x, chunkpos->y));
		HashMap_remove(world->scheduledMeshUpdateChunks, Chunk_hash_pos(chunkpos->x, chunkpos->y));
		free(chunkPosNode);
		free(chunkpos);
		if (!c) return;
		Chunk c_l = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX - 1, c->posZ));
		Chunk c_r = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX + 1, c->posZ));
		Chunk c_b = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX, c->posZ - 1));
		Chunk c_f = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX, c->posZ + 1));
		if (c_l && c_r && c_b && c_f) {
			Chunk neighbors[4] = { c_l, c_r, c_b, c_f };
			Chunk_updatemesh(c, neighbors);
		}
	}

	for (int i = 0; i < world->chunks->count; i++) { //schedule mesh update of unmeshed chunks with loaded neighbors
		Chunk c = (Chunk)List_get(world->chunks, i);
		if (!c || c->setMesh) continue;
		if (HashMap_containsKey(world->scheduledMeshUpdateChunks, Chunk_hash(c))) continue;
		Chunk c_l = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX - 1, c->posZ));
		Chunk c_r = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX + 1, c->posZ));
		Chunk c_b = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX, c->posZ - 1));
		Chunk c_f = HashMap_get(world->chunkmap, Chunk_hash_pos(c->posX, c->posZ + 1));
		if (c_l && c_r && c_b && c_f) {
			Chunk neighbors[4] = { c_l, c_r, c_b, c_f };
			Chunk_updatemesh(c, neighbors);
		}
	}

	//camera moved between chunks
	if (camX != prevCamX || camZ != prevCamZ) {
		for (int i = 0; i < world->chunks->count; i++) { //unload chunks out of render distance
			Chunk c = (Chunk)List_get(world->chunks, i);
			if (!c) continue;
			if (sqrt((double)((c->posX - camX) * (c->posX - camX) + (c->posZ - camZ) * (c->posZ - camZ))) > RENDER_DISTANCE) {
				List_remove(world->chunks, i);
				HashMap_remove(world->chunkmap, Chunk_hash(c));
				HashMap_remove_free(world->scheduledLoadChunks, Int2_free, Chunk_hash(c));
				Chunk_free(c);
			}
		}

		for (int i = 1; i < RENDER_DISTANCE; i++) {
			for (int x = -i + camX; x <= i + camX; x++) { //schedule load of new chunks
				for (int z = -i + camZ; z <= i + camZ; z++) {
					double camDistance = sqrt((double)((x - camX) * (x - camX) + (z - camZ) * (z - camZ)));
					if ((int)floor(camDistance) > i) continue;
					if (HashMap_get(world->chunkmap, Chunk_hash_pos(x, z))) continue;
					if (HashMap_get(world->scheduledLoadChunks, Chunk_hash_pos(x, z))) continue;
					LinkedList_prepend(world->chunkLoadQueue, new_Int2(x, z));
					HashMap_insert_at(world->scheduledLoadChunks, Chunk_hash_pos(x, z), new_Int2(x, z));
				}
			}
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
		c->cull = Vec3_dot(cvec, camForward) > 0.15f && dist > 40.0f;
		free(cvec);
	}
	free(camForward);
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
		if (!chunk) continue;
		if (chunk->cull && FRUSTUM_CULLING) continue;
		RenderComponent renderer = chunk->renderer;
		if (!renderer) continue;
		if (!renderer->transformStatic) RenderComponent_updatetransformmatrix(renderer);

		Shader_setMat4x4(world->chunkShader, 0, &renderer->transform->matrix->data[0][0]);
		glBindVertexArray(renderer->VAO);
		glDrawElements(GL_TRIANGLES, renderer->indexCount, GL_UNSIGNED_INT, 0);
	}
}
#pragma warning(disable: 6385)
#pragma warning(disable: 6386)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "chunk.h"
#include "atlas.h"
#include "math/noise.h"
#include "main.h"
#include "blockdef.h"

#define DEBUG_COLOR_CHUNK_BORDERS 0
static unsigned int LIGHT_AMT_TOP = 15;
static unsigned int LIGHT_AMT_BOTTOM = 9;
static unsigned int LIGHT_AMT_FRONT = 12;
static unsigned int LIGHT_AMT_BACK = 10;
static unsigned int LIGHT_AMT_RIGHT = 11;
static unsigned int LIGHT_AMT_LEFT = 14;

unsigned int createCube();
Chunk new_Chunk() {
	Chunk this = (Chunk)malloc(sizeof(struct Chunk));
	if (!this) return NULL;

	this->renderer = new_RenderComponent(NULL, 0, 0, 0, 1);
	this->setMesh = 0;
	this->cull = 0;

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			for (int y = 0; y < 256; y++) {
				this->data[x][z][y] = (char)0;
			}
		}
	}
	return this;
}

void Chunk_free(Chunk chunk) {
	if (!chunk) return;
	//printf("(% d, % d)\n", chunk->posX, chunk->posZ);
	//if (!chunk->setMesh) printf("mesh not set (%d, %d)\n", chunk->posX, chunk->posZ);
	//unbind VAO, VBO, and EBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//delete VAO, VBO, and EBO
	glDeleteVertexArrays(1, &chunk->renderer->VAO);
	glDeleteBuffers(1, &chunk->renderer->VBO);
	glDeleteBuffers(1, &chunk->renderer->EBO);

	Shader_free(chunk->renderer->shader);
	RenderComponent_free(chunk->renderer);
	free(chunk);
}

int Chunk_hash(Chunk c) {
	return Chunk_hash_pos(c->posX, c->posZ);
}
int Chunk_hash_pos(int x, int z) {
	/*
	int hashX = (unsigned short)x * 2 - (x < 0 ? 1 : 0);
	int hashY = (unsigned short)z * 2 - (z < 0 ? 1 : 0);
	unsigned int hash = (unsigned int)((unsigned short)hashX << 16) | (unsigned short)(hashY);*/

	int a = (x >= 0 ? 2 * x : -2 * x - 1);
	int b = (z >= 0 ? 2 * z : -2 * z - 1);
	return ((a + b) * (a + b + 1) / 2 + b);
}

Chunk Chunk_generate(World world, int xPos, int zPos) {
	Chunk chunk = new_Chunk();
	if (!chunk) {
		printf("Couldn't generate chunk\n");
		return NULL;
	}
	chunk->posX = xPos;
	chunk->posZ = zPos;
	chunk->renderer->transform->position->x = 16.0f * (float)xPos;
	chunk->renderer->transform->position->z = 16.0f * (float)zPos;
	RenderComponent_updatetransformmatrix(chunk->renderer);

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			double _x = (double)(x + 16 * xPos); 
			double _z = (double)(z + 16 * zPos);
			double oceanMapVal = octaveNoise(world->oceanMap, _x, _z);
			double oceanBlendFactor = atan(90.0 * (oceanMapVal - 0.3)) * 1.02 / M_PI + 0.51;
			oceanBlendFactor = fmin(1.0, fmax(0.0, oceanBlendFactor));
			double oceanVal = octaveNoise(world->biomeInfo[BIOME_OCEAN]->heightmap, _x, _z);
			double groundVal = octaveNoise(world->biomeInfo[BIOME_PLAINS]->heightmap, _x, _z);
			int height = (int)(oceanVal + (groundVal - oceanVal) * oceanBlendFactor);

			for (int y = 0; y < 256; y++) {
				if (y < height - 10) {
					chunk->data[x][z][y] = (char)BLOCK_STONE;
				}
				else if (y < height) {
					if (oceanMapVal > 0.34 || (oceanMapVal >= 0.34 && y < 62)) chunk->data[x][z][y] = (char)BLOCK_DIRT;
					else if (oceanMapVal < 0.34 || y < 66) chunk->data[x][z][y] = (char)BLOCK_SAND;
					else chunk->data[x][z][y] = (char)BLOCK_STONE;
				}
				else if (y == height) {
					if (oceanMapVal < 0.3025 || (oceanMapVal >= 0.3025 && y < 62)) chunk->data[x][z][y] = (char)BLOCK_STONE;
					else if (oceanMapVal < 0.34 || y < 66) chunk->data[x][z][y] = (char)BLOCK_SAND;
					else chunk->data[x][z][y] = (char)BLOCK_GRASS;
				}
			}
		}
	}
	return chunk;
}

void Chunk_updatemesh(Chunk chunk, Chunk neighboringData[4]) {
	int blockCt = 0;
	int faceCt = 0;
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			for (int y = 0; y < 256; y++) {
				if (!(int)chunk->data[x][z][y]) {
					if (y == 63) { //water face
						faceCt++;
					}
					continue;
				}
				blockCt++;
				faceCt += 6;
				if (x > 0 && (int)chunk->data[x - 1][z][y] || x == 0 && (int)neighboringData[0]->data[15][z][y]) faceCt--;
				if (x < 15 && (int)chunk->data[x + 1][z][y] || x == 15 && (int)neighboringData[1]->data[0][z][y]) faceCt--;
				if (z > 0 && (int)chunk->data[x][z - 1][y] || z == 0 && (int)neighboringData[2]->data[x][15][y]) faceCt--;
				if (z < 15 && (int)chunk->data[x][z + 1][y] || z == 15 && (int)neighboringData[3]->data[x][0][y]) faceCt--;
				if (y > 0 && (int)chunk->data[x][z][y + 1]) faceCt--;
				if (y < 255 && (int)chunk->data[x][z][y - 1]) faceCt--;
			}
		}
	}
	float* vertices = (float*)malloc(faceCt * 4 * sizeof(float)); //4 vertices per face
	unsigned int* indices = (unsigned int*)malloc(faceCt * 6 * sizeof(unsigned int)); //6 indicies per face
	int block = 0;
	int face = 0;

	//generate main opaque blocks
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			for (int y = 0; y < 256; y++) {
				int currentBlockType = (int)chunk->data[x][z][y];
				if (!currentBlockType) continue;

				GLubyte _x = x; 
				GLubyte _y = y;
				GLubyte _z = z;
				int i = face * 6;
				int v = face * 4;
				if (y == 255 || !(int)chunk->data[x][z][y + 1]) { //top face
					
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_TOP);

					vertices[v + 0] = (float)((_x + 0) | ((_y + 1) << 5u) | ((_z + 1) << 14u) | (texCoords[0] << 19u));
					vertices[v + 1] = (float)((_x + 1) | ((_y + 1) << 5u) | ((_z + 1) << 14u) | (texCoords[1] << 19u));
					vertices[v + 2] = (float)((_x + 1) | ((_y + 1) << 5u) | ((_z + 0) << 14u) | (texCoords[2] << 19u));
					vertices[v + 3] = (float)((_x + 0) | ((_y + 1) << 5u) | ((_z + 0) << 14u) | (texCoords[3] << 19u));

					free(texCoords);

					indices[i + 0] = v + 0; indices[i + 1] = v + 1; indices[i + 2] = v + 2;
					indices[i + 3] = v + 0; indices[i + 4] = v + 2; indices[i + 5] = v + 3;
					i += 6;
					v += 4;
					face++;
				}
				if (y == 0 || !(int)chunk->data[x][z][y - 1]) { //bottom face
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_BOTTOM);

					vertices[v + 0] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[0] << 19));
					vertices[v + 1] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[1] << 19));
					vertices[v + 2] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[2] << 19));
					vertices[v + 3] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[3] << 19));

					free(texCoords);

					indices[i + 0] = v + 0; indices[i + 1] = v + 2; indices[i + 2] = v + 1;
					indices[i + 3] = v + 0; indices[i + 4] = v + 3; indices[i + 5] = v + 2;
					i += 6;
					v += 4;
					face++;
				}
				if (x > 0 && !(int)chunk->data[x - 1][z][y] || x == 0 && !(int)neighboringData[0]->data[15][z][y]) { //left face
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_LEFT);

					vertices[v + 0] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[0] << 19));
					vertices[v + 1] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[1] << 19));
					vertices[v + 2] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[2] << 19));
					vertices[v + 3] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[3] << 19));

					free(texCoords);

					indices[i + 0] = v + 0; indices[i + 1] = v + 2; indices[i + 2] = v + 1;
					indices[i + 3] = v + 0; indices[i + 4] = v + 3; indices[i + 5] = v + 2;
					i += 6;
					v += 4;
					face++;
				}
				if (x < 15 && !(int)chunk->data[x + 1][z][y] || x == 15 && !(int)neighboringData[1]->data[0][z][y]) { //right face
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_RIGHT);

					vertices[v + 0] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[0] << 19));
					vertices[v + 1] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[1] << 19));
					vertices[v + 2] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[2] << 19));
					vertices[v + 3] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[3] << 19));

					free(texCoords);

					indices[i + 0] = v + 0; indices[i + 1] = v + 1; indices[i + 2] = v + 2;
					indices[i + 3] = v + 0; indices[i + 4] = v + 2; indices[i + 5] = v + 3;
					i += 6;
					v += 4;
					face++;
				}
				if (z < 15 && !(int)chunk->data[x][z + 1][y] || z == 15 && !(int)neighboringData[3]->data[x][0][y]) { //front face
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_FRONT);

					vertices[v + 0] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[0] << 19));
					vertices[v + 1] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[1] << 19));
					vertices[v + 2] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[2] << 19));
					vertices[v + 3] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 1) << 14) | (texCoords[3] << 19));

					free(texCoords);
					
					indices[i + 0] = v + 0; indices[i + 1] = v + 2; indices[i + 2] = v + 1;
					indices[i + 3] = v + 0; indices[i + 4] = v + 3; indices[i + 5] = v + 2;
					i += 6;
					v += 4;
					face++;
				}
				if (z > 0 && !(int)chunk->data[x][z - 1][y] || z == 0 && !(int)neighboringData[2]->data[x][15][y]) { //back face
					GLubyte* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_BACK);

					vertices[v + 0] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[0] << 19));
					vertices[v + 1] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[1] << 19));
					vertices[v + 2] = (float)((_x + 1) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[2] << 19));
					vertices[v + 3] = (float)((_x + 0) | ((_y + 0) << 5) | ((_z + 0) << 14) | (texCoords[3] << 19));

					free(texCoords);

					indices[i + 0] = v + 0; indices[i + 1] = v + 1; indices[i + 2] = v + 2;
					indices[i + 3] = v + 0; indices[i + 4] = v + 2; indices[i + 5] = v + 3;
					i += 6;
					v += 4;
					face++;
				}
				block++;
			}
		}
	}
	//generate water faces
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			if (chunk->data[x][z][63]) continue;
			GLubyte _x = x;
			GLubyte _y = 63;
			GLubyte _z = z;

			int i = face * 6;
			int v = face * 4;
			int vertex = face * 4;

			GLubyte* texCoords = TextureAtlas_getFaceCoords(BLOCK_WATER, BLOCKFACE_TOP);

			vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.4f; vertices[v + 2] = _z + 0.5f;
			vertices[v + 5] = _x + 0.5f; vertices[v + 6] = _y + 0.4f; vertices[v + 7] = _z + 0.5f;
			vertices[v + 10] = _x + 0.5f; vertices[v + 11] = _y + 0.4f; vertices[v + 12] = _z - 0.5f;
			vertices[v + 15] = _x - 0.5f; vertices[v + 16] = _y + 0.4f; vertices[v + 17] = _z - 0.5f;

			vertices[v + 0] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[0] << 19));
			vertices[v + 1] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 1) << 14) | (texCoords[1] << 19));
			vertices[v + 2] = (float)((_x + 1) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[2] << 19));
			vertices[v + 3] = (float)((_x + 0) | ((_y + 1) << 5) | ((_z + 0) << 14) | (texCoords[3] << 19));

			free(texCoords);

			indices[i + 0] = v + 0; indices[i + 1] = v + 1; indices[i + 2] = v + 2;
			indices[i + 3] = v + 0; indices[i + 4] = v + 2; indices[i + 5] = v + 3;

			face++;
		}
	}
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	unsigned int VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * faceCt, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 * faceCt, indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void*)0); //vertex positions
	glEnableVertexAttribArray(0);
	chunk->renderer->VAO = VAO;
	chunk->renderer->VBO = VBO;
	chunk->renderer->EBO = EBO;
	chunk->renderer->indexCount = faceCt * 6;

	free(vertices);
	free(indices);

	chunk->setMesh = 1;
}

unsigned int createCube() {
	unsigned int result = 0;

	//square VBO/EBO
	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
	};
	unsigned int indices[] = {
		//Right
		1, 3, 7,
		1, 5, 7,
		//Front
		0, 2, 3,
		0, 1, 3,
		//Back
		4, 6, 7,
		4, 5, 7
	};
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glGenVertexArrays(1, &result);
	glBindVertexArray(result);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //vertex colors
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); //vertex tex coords
	glEnableVertexAttribArray(2);
	return result;
}
#pragma warning(disable: 6385)
#pragma warning(disable: 6386)

#include <stdlib.h>
#include <stdio.h>
#include "chunk.h"
#include "atlas.h"
#include "math/noise.h"
#include "main.h"

#define LIGHT_AMT_TOP 1.0
#define LIGHT_AMT_BOTTOM 0.6
#define LIGHT_AMT_FRONT 0.8
#define LIGHT_AMT_BACK 0.6
#define LIGHT_AMT_RIGHT 0.7
#define LIGHT_AMT_LEFT 0.9
#define DEBUG_COLOR_CHUNK_BORDERS 0

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


Chunk Chunk_generate(int xPos, int zPos) {
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

	double baseHeight = 40.0;
	double amplitude = 60.0;
	double freq = 0.0075;

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			int height = (int)((double)amplitude * octaveNoise(((double)x + 16.0 * (double)xPos) * freq, ((double)z + 16.0 * (double)zPos) * freq) + baseHeight);
			//printf("%d\n", height);
			for (int y = 0; y < 256; y++) {
				if (y < height - 10) {
					chunk->data[x][z][y] = (char)BLOCK_STONE;
				}
				else if (y < height) {
					chunk->data[x][z][y] = (char)BLOCK_DIRT;
				}
				else if (y == height) {
					chunk->data[x][z][y] = (char)BLOCK_GRASS;
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
	float* vertices = (float*)malloc(faceCt * 32 * sizeof(float)); //8 floats, 4 vertices per face
	unsigned int* indices = (unsigned int*)malloc(faceCt * 6 * sizeof(unsigned int)); //6 indicies per face
	int block = 0;
	int face = 0;

	//generate main opaque blocks
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			for (int y = 0; y < 256; y++) {
				int currentBlockType = (int)chunk->data[x][z][y];
				if (!currentBlockType) continue;
				float _x = (float)x; float _y = (float)y; float _z = (float)z;

				//vertex colors
				float colX, colY, colZ;
				colX = colY = colZ = 1.0f;

				int i = face * 6;
				int v = face * 32;
				int vertex = face * 4;
				if (y == 255 || !(int)chunk->data[x][z][y + 1]) { //top face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_TOP);
					for (int k = 0; k < 4; k++) { 
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_TOP * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_TOP * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_TOP * colZ;

						if (DEBUG_COLOR_CHUNK_BORDERS && (x == 0 || x == 15 || z == 0 || z == 15)) {
							vertices[v + k * 8 + 3] = 0.0;
							vertices[v + k * 8 + 4] = 0.0;
							vertices[v + k * 8 + 5] = 0.0;
						}

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.5f; vertices[v + 2] = _z + 0.5f;
					vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y + 0.5f; vertices[v + 10] = _z + 0.5f;
					vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y + 0.5f; vertices[v + 18] = _z - 0.5f;
					vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y + 0.5f; vertices[v + 26] = _z - 0.5f;
					
					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 1; indices[i + 2] = vertex + 2;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 2; indices[i + 5] = vertex + 3;
					i += 6;
					v += 32;
					vertex += 4;
					face++;
				}
				if (y == 0 || !(int)chunk->data[x][z][y - 1]) { //bottom face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_BOTTOM);
					for (int k = 0; k < 4; k++) {
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_BOTTOM * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_BOTTOM * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_BOTTOM * colZ;

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y - 0.5f; vertices[v + 2] = _z + 0.5f;
					vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y - 0.5f; vertices[v + 10] = _z + 0.5f;
					vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y - 0.5f; vertices[v + 18] = _z - 0.5f;
					vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y - 0.5f; vertices[v + 26] = _z - 0.5f;

					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 2; indices[i + 2] = vertex + 1;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 3; indices[i + 5] = vertex + 2;
					i += 6;
					v += 32;
					vertex += 4;
					face++;
				}
				if (x > 0 && !(int)chunk->data[x - 1][z][y] || x == 0 && !(int)neighboringData[0]->data[15][z][y]) { //left face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_LEFT);
					for (int k = 0; k < 4; k++) {
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_LEFT * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_LEFT * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_LEFT * colZ;

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.5f; vertices[v + 2] = _z - 0.5f;
					vertices[v + 8] = _x - 0.5f; vertices[v + 9] = _y + 0.5f; vertices[v + 10] = _z + 0.5f;
					vertices[v + 16] = _x - 0.5f; vertices[v + 17] = _y - 0.5f; vertices[v + 18] = _z + 0.5f;
					vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y - 0.5f; vertices[v + 26] = _z - 0.5f;

					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 2; indices[i + 2] = vertex + 1;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 3; indices[i + 5] = vertex + 2;
					i += 6;
					v += 32;
					vertex += 4;
					face++;
				}
				if (x < 15 && !(int)chunk->data[x + 1][z][y] || x == 15 && !(int)neighboringData[1]->data[0][z][y]) { //right face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_RIGHT);
					for (int k = 0; k < 4; k++) {
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_RIGHT * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_RIGHT * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_RIGHT * colZ;

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x + 0.5f; vertices[v + 1] = _y + 0.5f; vertices[v + 2] = _z - 0.5f;
					vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y + 0.5f; vertices[v + 10] = _z + 0.5f;
					vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y - 0.5f; vertices[v + 18] = _z + 0.5f;
					vertices[v + 24] = _x + 0.5f; vertices[v + 25] = _y - 0.5f; vertices[v + 26] = _z - 0.5f;

					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 1; indices[i + 2] = vertex + 2;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 2; indices[i + 5] = vertex + 3;
					i += 6;
					v += 32;
					vertex += 4;
					face++;
				}
				if (z < 15 && !(int)chunk->data[x][z + 1][y] || z == 15 && !(int)neighboringData[3]->data[x][0][y]) { //front face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_FRONT);
					for (int k = 0; k < 4; k++) {
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_FRONT * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_FRONT * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_FRONT * colZ;

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.5f; vertices[v + 2] = _z + 0.5f;
					vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y + 0.5f; vertices[v + 10] = _z + 0.5f;
					vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y - 0.5f; vertices[v + 18] = _z + 0.5f;
					vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y - 0.5f; vertices[v + 26] = _z + 0.5f;

					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 2; indices[i + 2] = vertex + 1;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 3; indices[i + 5] = vertex + 2;
					i += 6;
					v += 32;
					vertex += 4;
					face++;
				}
				if (z > 0 && !(int)chunk->data[x][z - 1][y] || z == 0 && !(int)neighboringData[2]->data[x][15][y]) { //back face
					float* texCoords = TextureAtlas_getFaceCoords(currentBlockType, BLOCKFACE_BACK);
					for (int k = 0; k < 4; k++) {
						//color
						vertices[v + k * 8 + 3] = LIGHT_AMT_BACK * colX;
						vertices[v + k * 8 + 4] = LIGHT_AMT_BACK * colY;
						vertices[v + k * 8 + 5] = LIGHT_AMT_BACK * colZ;

						//tex coordinates
						vertices[v + k * 8 + 6] = texCoords[k * 2];
						vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
					}
					free(texCoords);

					vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.5f; vertices[v + 2] = _z - 0.5f;
					vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y + 0.5f; vertices[v + 10] = _z - 0.5f;
					vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y - 0.5f; vertices[v + 18] = _z - 0.5f;
					vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y - 0.5f; vertices[v + 26] = _z - 0.5f;

					indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 1; indices[i + 2] = vertex + 2;
					indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 2; indices[i + 5] = vertex + 3;
					i += 6;
					v += 32;
					vertex += 4;
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
			float _x = (float)x; float _y = 63.0; float _z = (float)z;

			//vertex colors
			float colX, colY, colZ;
			colX = colY = colZ = 1.0f;

			int i = face * 6;
			int v = face * 32;
			int vertex = face * 4;

			float* texCoords = TextureAtlas_getFaceCoords(BLOCK_WATER, BLOCKFACE_TOP);
			for (int k = 0; k < 4; k++) {
				//color
				vertices[v + k * 8 + 3] = LIGHT_AMT_TOP * colX;
				vertices[v + k * 8 + 4] = LIGHT_AMT_TOP * colY;
				vertices[v + k * 8 + 5] = LIGHT_AMT_TOP * colZ;

				//tex coordinates
				vertices[v + k * 8 + 6] = texCoords[k * 2];
				vertices[v + k * 8 + 7] = texCoords[k * 2 + 1];
			}
			free(texCoords);

			vertices[v + 0] = _x - 0.5f; vertices[v + 1] = _y + 0.4f; vertices[v + 2] = _z + 0.5f;
			vertices[v + 8] = _x + 0.5f; vertices[v + 9] = _y + 0.4f; vertices[v + 10] = _z + 0.5f;
			vertices[v + 16] = _x + 0.5f; vertices[v + 17] = _y + 0.4f; vertices[v + 18] = _z - 0.5f;
			vertices[v + 24] = _x - 0.5f; vertices[v + 25] = _y + 0.4f; vertices[v + 26] = _z - 0.5f;

			indices[i + 0] = vertex + 0; indices[i + 1] = vertex + 1; indices[i + 2] = vertex + 2;
			indices[i + 3] = vertex + 0; indices[i + 4] = vertex + 2; indices[i + 5] = vertex + 3;
			i += 6;
			v += 32;
			vertex += 4;
			face++;
		}
	}

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	unsigned int vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 32 * faceCt, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 * faceCt, indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //vertex colors
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); //vertex tex coords
	glEnableVertexAttribArray(2);
	chunk->renderer->VAO = vao;
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
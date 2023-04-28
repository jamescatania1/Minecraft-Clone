#pragma warning(disable: 6011)

#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>
#include "atlas.h"

#define ATLAS_CELL_WIDTH 16
#define ATLAS_CELL_HEIGHT 16

//block atlas coordinates -- top left is (0, 0)
#define ATLAS_BLOCK_GRASS_TOP_X 0
#define ATLAS_BLOCK_GRASS_TOP_Y 0

#define ATLAS_BLOCK_GRASS_SIDE_X 1
#define ATLAS_BLOCK_GRASS_SIDE_Y 0

#define ATLAS_BLOCK_DIRT_X 2
#define ATLAS_BLOCK_DIRT_Y 0

#define ATLAS_BLOCK_STONE_X 3
#define ATLAS_BLOCK_STONE_Y 0

#define ATLAS_BLOCK_WATER_X 4
#define ATLAS_BLOCK_WATER_Y 0


unsigned int atlas;
int width, height, nrChannels;
float cellWidth;
float cellHeight;

//[blocktype][blockface][x/y]
float texCoords[128][6][2];

void TextureAtlas_init() {
	//load texture and generate it for openGL
	unsigned char* data = stbi_load("content\\atlas.png", &width, &height, &nrChannels, 0);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &atlas);
	glBindTexture(GL_TEXTURE_2D, atlas);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Disable mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else printf("Failed to read atlas data\n");
	free(data);

	cellWidth = (float)ATLAS_CELL_WIDTH / (float)width;
	cellHeight = (float)ATLAS_CELL_HEIGHT / (float)height;

	//grass block faces
	texCoords[BLOCK_GRASS][BLOCKFACE_TOP][0] = ATLAS_BLOCK_GRASS_TOP_X;
	texCoords[BLOCK_GRASS][BLOCKFACE_TOP][1] = ATLAS_BLOCK_GRASS_TOP_Y;
	texCoords[BLOCK_GRASS][BLOCKFACE_BOTTOM][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_GRASS][BLOCKFACE_BOTTOM][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_GRASS][BLOCKFACE_FRONT][0] = ATLAS_BLOCK_GRASS_SIDE_X; 
	texCoords[BLOCK_GRASS][BLOCKFACE_FRONT][1] = ATLAS_BLOCK_GRASS_SIDE_Y;
	texCoords[BLOCK_GRASS][BLOCKFACE_BACK][0] = ATLAS_BLOCK_GRASS_SIDE_X;
	texCoords[BLOCK_GRASS][BLOCKFACE_BACK][1] = ATLAS_BLOCK_GRASS_SIDE_Y;
	texCoords[BLOCK_GRASS][BLOCKFACE_LEFT][0] = ATLAS_BLOCK_GRASS_SIDE_X;
	texCoords[BLOCK_GRASS][BLOCKFACE_LEFT][1] = ATLAS_BLOCK_GRASS_SIDE_Y;
	texCoords[BLOCK_GRASS][BLOCKFACE_RIGHT][0] = ATLAS_BLOCK_GRASS_SIDE_X;
	texCoords[BLOCK_GRASS][BLOCKFACE_RIGHT][1] = ATLAS_BLOCK_GRASS_SIDE_Y;

	//dirt block faces
	texCoords[BLOCK_DIRT][BLOCKFACE_TOP][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_TOP][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_DIRT][BLOCKFACE_BOTTOM][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_BOTTOM][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_DIRT][BLOCKFACE_FRONT][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_FRONT][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_DIRT][BLOCKFACE_BACK][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_BACK][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_DIRT][BLOCKFACE_LEFT][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_LEFT][1] = ATLAS_BLOCK_DIRT_Y;
	texCoords[BLOCK_DIRT][BLOCKFACE_RIGHT][0] = ATLAS_BLOCK_DIRT_X; 
	texCoords[BLOCK_DIRT][BLOCKFACE_RIGHT][1] = ATLAS_BLOCK_DIRT_Y;

	//dirt block faces
	texCoords[BLOCK_STONE][BLOCKFACE_TOP][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_TOP][1] = ATLAS_BLOCK_STONE_Y;
	texCoords[BLOCK_STONE][BLOCKFACE_BOTTOM][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_BOTTOM][1] = ATLAS_BLOCK_STONE_Y;
	texCoords[BLOCK_STONE][BLOCKFACE_FRONT][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_FRONT][1] = ATLAS_BLOCK_STONE_Y;
	texCoords[BLOCK_STONE][BLOCKFACE_BACK][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_BACK][1] = ATLAS_BLOCK_STONE_Y;
	texCoords[BLOCK_STONE][BLOCKFACE_LEFT][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_LEFT][1] = ATLAS_BLOCK_STONE_Y;
	texCoords[BLOCK_STONE][BLOCKFACE_RIGHT][0] = ATLAS_BLOCK_STONE_X;
	texCoords[BLOCK_STONE][BLOCKFACE_RIGHT][1] = ATLAS_BLOCK_STONE_Y;

	//dirt block faces
	texCoords[BLOCK_WATER][BLOCKFACE_TOP][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_TOP][1] = ATLAS_BLOCK_WATER_Y;
	texCoords[BLOCK_WATER][BLOCKFACE_BOTTOM][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_BOTTOM][1] = ATLAS_BLOCK_WATER_Y;
	texCoords[BLOCK_WATER][BLOCKFACE_FRONT][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_FRONT][1] = ATLAS_BLOCK_WATER_Y;
	texCoords[BLOCK_WATER][BLOCKFACE_BACK][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_BACK][1] = ATLAS_BLOCK_WATER_Y;
	texCoords[BLOCK_WATER][BLOCKFACE_LEFT][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_LEFT][1] = ATLAS_BLOCK_WATER_Y;
	texCoords[BLOCK_WATER][BLOCKFACE_RIGHT][0] = ATLAS_BLOCK_WATER_X;
	texCoords[BLOCK_WATER][BLOCKFACE_RIGHT][1] = ATLAS_BLOCK_WATER_Y;

	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 6; j++) {
			texCoords[i][j][0] *= cellWidth;
			texCoords[i][j][1] *= cellHeight;
		}
	}
}

void TextureAtlas_free() {

}

float* TextureAtlas_getFaceCoords(BLOCK_TYPE block, ATLAS_BLOCKFACE face) {
	float* result = (float*)malloc(8 * sizeof(float));
	result[0] = texCoords[block][face][0];  //top left
	result[1] = texCoords[block][face][1];
	result[2] = result[0] + cellWidth;		//top right
	result[3] = result[1];
	result[4] = result[2];				    //bottom right
	result[5] = result[1] + cellWidth;
	result[6] = result[0];					//bottom left
	result[7] = result[5];
	return result;
}

unsigned int TextureAtlas_currentTexture() {
	return atlas;
}
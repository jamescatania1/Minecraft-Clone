#pragma warning(disable: 6011)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>
#include "atlas.h"
#include "structures/list.h"

#define ATLAS_CELL_WIDTH 16
#define ATLAS_CELL_HEIGHT 16

unsigned int atlas;
int width, height, nrChannels;
float cellWidth;
float cellHeight;

//[blocktype]
BlockInfo blockData[128];

void parseBlockInfo();

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

	//parse blockinfo file and populate blockData
	parseBlockInfo();

	cellWidth = (float)ATLAS_CELL_WIDTH / (float)width;
	cellHeight = (float)ATLAS_CELL_HEIGHT / (float)height;

	for (int i = 0; i < 128; i++) {
		if (!blockData[i]) continue;
		for (int j = 0; j < 6; j++) {
			blockData[i]->faces[j]->x *= cellWidth;
			blockData[i]->faces[j]->y *= cellHeight;
		}
	}
}

void TextureAtlas_free() {
	for (int i = 0; i < 128; i++) {
		if (!blockData[i]) continue;
		BlockInfo_free(blockData[i]);
	}
}

void BlockInfo_free(BlockInfo block) {
	for (int i = 0; i < 6; i++) {
		free(block->faces[i]);
	}
	free(block);
}

float* TextureAtlas_getFaceCoords(BLOCK_TYPE block, ATLAS_BLOCKFACE face) {
	float* result = (float*)malloc(8 * sizeof(float));

	// if random orientation
	int r = blockData[block]->faces[face]->randRotation ? rand() % 4 : 0;
	if (r == 0) {
		result[0] = blockData[block]->faces[face]->x;  //top left
		result[1] = blockData[block]->faces[face]->y;
		result[2] = result[0] + cellWidth;		//top right
		result[3] = result[1];
		result[4] = result[2];				    //bottom right
		result[5] = result[1] + cellWidth;
		result[6] = result[0];					//bottom left
		result[7] = result[5];
	}
	else if (r == 1) {
		result[2] = blockData[block]->faces[face]->x;  //top left
		result[3] = blockData[block]->faces[face]->y;
		result[4] = result[2] + cellWidth;		//top right
		result[5] = result[3];
		result[6] = result[4];				    //bottom right
		result[7] = result[3] + cellWidth;
		result[0] = result[2];					//bottom left
		result[1] = result[7];
	}
	else if (r == 2) {
		result[4] = blockData[block]->faces[face]->x;  //top left
		result[5] = blockData[block]->faces[face]->y;
		result[6] = result[4] + cellWidth;		//top right
		result[7] = result[5];
		result[0] = result[6];				    //bottom right
		result[1] = result[5] + cellWidth;
		result[2] = result[4];					//bottom left
		result[3] = result[1];
	}
	else if (r == 3) {
		result[6] = blockData[block]->faces[face]->x;  //top left
		result[7] = blockData[block]->faces[face]->y;
		result[0] = result[6] + cellWidth;		//top right
		result[1] = result[7];
		result[2] = result[0];				    //bottom right
		result[3] = result[7] + cellWidth;
		result[4] = result[6];					//bottom left
		result[5] = result[3];
	}

	//flip x randomly
	r = blockData[block]->faces[face]->randFlipX ? rand() % 2 : 0;
	if (r) { //permutation (01)(23)
		float tx = result[0]; float ty = result[1];
		result[0] = result[2];
		result[1] = result[3];
		result[2] = tx; result[3] = ty;
		tx = result[4]; ty = result[5];
		result[4] = result[6]; 
		result[5] = result[7];
		result[6] = tx; result[7] = ty;
	}

	//flip y randomly
	r = blockData[block]->faces[face]->randFlipY ? rand() % 2 : 0;
	if (r) { //permutation (03)(12)
		float tx = result[0]; float ty = result[1];
		result[0] = result[6];
		result[1] = result[7];
		result[6] = tx; result[7] = ty;
		tx = result[2]; ty = result[3];
		result[2] = result[4];
		result[3] = result[5];
		result[4] = tx; result[5] = ty;
	}

	return result;
}

unsigned int TextureAtlas_currentTexture() {
	return atlas;
}

int parseInt(char* file, int i, List defines, List defineVals);
char* fileread(const char* filename);
void parseBlockInfo() {
	char* file = fileread("content\\atlas.blockinfo");
	List defines = new_List();
	List defineVals = new_List();
	int filelen = strlen(file);
	int i = 0;
	while (i < filelen) {
		if (file[i] == '#') { //define identifier
			i++;
			int j = i;
			while (file[j] != ' ') j++;
			char* defineStr = (char*)malloc((j - i + 1) * sizeof(char));
			for (int _j = i; _j < j; _j++) {
				defineStr[_j - i] = file[_j];
			}
			defineStr[j - i] = '\0';
			List_add(defines, defineStr);

			i = j = j + 1;
			while (file[j] != ';') j++;
			char* defineVal = (char*)malloc((j - i + 1) * sizeof(char));
			for (int _j = i; _j < j; _j++) {
				defineVal[_j - i] = file[_j];
			}
			defineVal[j - i] = '\0';
			int* defineValInt = (int*)malloc(sizeof(int));
			*defineValInt = atoi(defineVal);
			free(defineVal);
			List_add(defineVals, defineValInt);
			i = j;
			continue;
		}

		if (file[i] == '$') { //block identifier
			i++;
			while (file[i] == ' ') i++;
			BlockInfo info = (BlockInfo)malloc(sizeof(struct BlockInfo));
			int blockIndex = parseInt(file, i, defines, defineVals);
			for (int side = 0; side < 6; side++) {
				info->faces[side] = (BlockFaceInfo)malloc(sizeof(struct BlockFaceInfo));
				while (file[i] != '{') { i++; }
				i++; 
				while (file[i] == ' ') { i++; }
				info->faces[side]->x = parseInt(file, i, defines, defineVals);
				while (file[i] != ',') { i++; }
				i++; 
				while (file[i] == ' ') { i++; }
				info->faces[side]->y = parseInt(file, i, defines, defineVals);
				while (file[i] != ',') { i++; }
				i++; 
				while (file[i] == ' ') { i++; }
				info->faces[side]->randRotation = parseInt(file, i, defines, defineVals);
				while (file[i] != ',') { i++; }
				i++; 
				while (file[i] == ' ') { i++; }
				info->faces[side]->randFlipX = parseInt(file, i, defines, defineVals);
				while (file[i] != ',') { i++; }
				i++; 
				while (file[i] == ' ') { i++; }
				info->faces[side]->randFlipY = parseInt(file, i, defines, defineVals);
				while (file[i] != '}') i++;
			}
			blockData[blockIndex] = info;
			continue;
		}
		i++;
	}
	free(file);
	List_freeAll(defines, free);
	List_freeAll(defineVals, free);
}
char* fileread(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		return NULL;
	}
	fseek(file, 0L, SEEK_END);
	long file_size = ftell(file);
	rewind(file);
	char* buffer = (char*)malloc(sizeof(char) * (file_size + 1));
	if (buffer == NULL) {
		fclose(file);
		fprintf(stderr, "Failed to allocate memory for file '%s'\n", filename);
		return NULL;
	}
	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size) {
		fclose(file);
		free(buffer);
		fprintf(stderr, "Failed to read file '%s'\n", filename);
		return NULL;
	}
	buffer[file_size] = '\0';
	fclose(file);
	return buffer;
}
int parseInt(char* file, int i, List defines, List defineVals) {
	if (file[i] == '&') { //define identifier
		i++;
		int j = i;
		while (file[j] != ',' && file[j] != ' ' && file[j] != '}' && file[j] != ';') j++;
		char* replaceStr = (char*)malloc((j - i + 1) * sizeof(char));
		for (int _j = i; _j < j; _j++) {
			replaceStr[_j - i] = file[_j];
		}
		replaceStr[j - i] = '\0';
		for (int k = 0; k < defines->count; k++) {
			char* cmpDefine = (char*)List_get(defines, k);
			if (!strcmp(cmpDefine, replaceStr)) {
				free(replaceStr);
				return *((int*)List_get(defineVals, k));
			}
		}
		free(replaceStr);
		return -1;
	}
	int j = i;
	while (file[j] != ',' && file[j] != ' ' && file[j] != '}' && file[j] != ';') j++;
	char* intString = (char*)malloc((j - i + 1) * sizeof(char));
	for (int _j = i; _j < j; _j++) {
		intString[_j - i] = file[_j];
	}
	intString[j - i] = '\0';
	int result = atoi(intString);
	free(intString);
	return result;
}
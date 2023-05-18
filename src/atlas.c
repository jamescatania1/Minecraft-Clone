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
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);

	int xCt = width / ATLAS_CELL_WIDTH;
	int yCt = height / ATLAS_CELL_HEIGHT;

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, ATLAS_CELL_WIDTH, ATLAS_CELL_HEIGHT, xCt * yCt, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (int i = 0; i < xCt * yCt; i++) {
		int xOffset = (i % xCt) * ATLAS_CELL_WIDTH;
		int yOffset = (i / yCt) * ATLAS_CELL_HEIGHT;

		GLubyte pixels[ATLAS_CELL_WIDTH * ATLAS_CELL_HEIGHT * 4];
		for (int y = 0; y < ATLAS_CELL_HEIGHT; y++) {
			for (int x = 0; x < ATLAS_CELL_WIDTH; x++) {
				int atlasX = xOffset + x;
				int atlasY = yOffset + y;
				int pixelIndex = (atlasY * width + atlasX) * 4;

				pixels[(y * ATLAS_CELL_HEIGHT + x) * 4] = data[pixelIndex];
				pixels[(y * ATLAS_CELL_HEIGHT + x) * 4 + 1] = data[pixelIndex + 1];
				pixels[(y * ATLAS_CELL_HEIGHT + x) * 4 + 2] = data[pixelIndex + 2];
				pixels[(y * ATLAS_CELL_HEIGHT + x) * 4 + 3] = data[pixelIndex + 3];
			}
		}

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}

	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	if (!data) printf("Failed to read atlas data\n");
	free(data);
	

	//parse blockinfo file and populate blockData
	parseBlockInfo();
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

GLubyte* TextureAtlas_getFaceCoords(BLOCK_TYPE block, ATLAS_BLOCKFACE face) {
	GLubyte* result = (GLubyte*)malloc(4 * sizeof(char));

	// if random orientation. denoting permutations in S_4 in disjoint cycle notation (zero-indexed).
	int r = blockData[block]->faces[face]->randRotation ? rand() % 4 : 0;
	if (r == 0) {
		//permutation ()
		result[0] = (GLubyte)(64 * 0);	//top left
		result[1] = (GLubyte)(64 * 1);	//top right
		result[2] = (GLubyte)(64 * 2);	//bottom right
		result[3] = (GLubyte)(64 * 3);	//bottom left
	}
	else if (r == 1) {
		//permutation (0123)
		result[0] = (GLubyte)(64 * 1);
		result[1] = (GLubyte)(64 * 2);
		result[2] = (GLubyte)(64 * 3);
		result[3] = (GLubyte)(64 * 0);
	}
	else if (r == 2) {
		//permutation (02)(13)
		result[0] = (GLubyte)(64 * 2);
		result[1] = (GLubyte)(64 * 3);
		result[2] = (GLubyte)(64 * 0);
		result[3] = (GLubyte)(64 * 1);
	}
	else if (r == 3) {
		//permutation (0321)
		result[0] = (GLubyte)(64 * 3);
		result[1] = (GLubyte)(64 * 0);
		result[2] = (GLubyte)(64 * 1);
		result[3] = (GLubyte)(64 * 2);
	}

	//flip x randomly
	r = blockData[block]->faces[face]->randFlipX ? rand() % 2 : 0;
	if (r) {
		//permutation (01)(23)
		GLubyte t0 = result[0]; GLubyte t2 = result[2];
		result[0] = result[1];
		result[1] = t0;
		result[2] = result[3];
		result[3] = t2;
	}

	//flip y randomly
	r = blockData[block]->faces[face]->randFlipY ? rand() % 2 : 0;
	if (r) { 
		//permutation (03)(12)
		GLubyte t0 = result[0]; GLubyte t1 = result[1];
		result[0] = result[3];
		result[3] = t0;
		result[1] = result[2];
		result[2] = t1;
	}

	for (int i = 0; i < 4; i++) 
		result[i] = (GLubyte)(result[i] + 8 * blockData[block]->faces[face]->y + blockData[block]->faces[face]->x);

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
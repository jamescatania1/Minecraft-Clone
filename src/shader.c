#pragma warning( disable: 6386 )
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shader.h"

//global defines are marked as:
//&GLOBAL
//#define DEFINE_STRING 0
//the defined value (which has no functional purpose, just to stop intellisense errors) must be one digit,
//while the actual define (stored in globalDefineValues) may be any char*
static const int globalDefineCt = 16;
const char* globalDefineStrings[16];
const char* globalDefineValues[16];
static const char* globalIdentifier = "GLOBAL";

void Shader_addGlobalDefine(const char* name, const char* value) {
	for (int i = 0; i < globalDefineCt; i++) {
		if (!globalDefineStrings[i]) {
			globalDefineStrings[i] = name;
			globalDefineValues[i] = value;
			break;
		}
	}
}

char* readFile(const char* filename) {
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

	//code for preprocessing my custom global defines
	//not by any means equipped to handle edge cases
	int preprocessedFileSize = file_size;
	for (int i = 0; i < file_size; i++) {
		if (buffer[i] == '&') {
			int validDefine = 1;
			for (int l = 0; l < 6; l++) {
				if (buffer[i + l + 1] != globalIdentifier[l]) {
					validDefine = 0;
					break;
				}
			}
			if (!validDefine) continue;
			while (buffer[i] != '#') i++;
			while (buffer[i] != ' ') i++;
			while (buffer[i] == ' ') i++;
			int defineNameSize = 0;
			while (buffer[i + defineNameSize] != ' ' && buffer[i + defineNameSize] != '\n') {
				defineNameSize++;
			}
			char* defineName = (char*)malloc((defineNameSize + 1) * sizeof(char));
			for (int j = 0; j < defineNameSize; j++) {
				defineName[j] = buffer[i + j];
			}
			defineName[defineNameSize] = '\0';
			for (int j = 0; j < globalDefineCt; j++) {
				if (!globalDefineStrings[j]) continue;
				if (!strcmp(globalDefineStrings[j], defineName)) {
					preprocessedFileSize += strlen(globalDefineValues[j]) - 1;
					break;
				}
			}
			free(defineName);
		}
	}
	char* pbuffer = (char*)malloc((preprocessedFileSize + 1) * sizeof(char));
	int poffset = 0;
	for (int i = 0; i < file_size; i++) {
		if (buffer[i] == '&') {
			int validDefine = 1;
			for (int l = 0; l < 6; l++) {
				if (buffer[i + l + 1] != globalIdentifier[l]) {
					validDefine = 0;
					break;
				}
			}
			if (!validDefine) {
				pbuffer[i + poffset] = buffer[i];
				continue;
			}
			while (buffer[i] != '#') {
				pbuffer[i + poffset] = buffer[i];
				i++;
			}
			while (buffer[i] != ' ') {
				pbuffer[i + poffset] = buffer[i];
				i++;
			}
			while (buffer[i] == ' ') {
				pbuffer[i + poffset] = buffer[i];
				i++;
			}
			int defineNameSize = 0;
			while (buffer[i + defineNameSize] != ' ' && buffer[i + defineNameSize] != '\n') {
				defineNameSize++;
			}
			char* defineName = (char*)malloc((defineNameSize + 1) * sizeof(char));
			for (int j = 0; j < defineNameSize; j++) {
				defineName[j] = buffer[i + j];
				pbuffer[i + poffset + j] = buffer[i + j];
			}
			i += defineNameSize;
			while (buffer[i] == ' ') {
				pbuffer[i + poffset] = buffer[i];
				i++;
			}

			defineName[defineNameSize] = '\0';
			for (int j = 0; j < globalDefineCt; j++) {
				if (!globalDefineStrings[j]) continue;
				if (!strcmp(globalDefineStrings[j], defineName)) {
					int defnLen = strlen(globalDefineValues[j]);
					for (int k = 0; k < defnLen; k++) {
						pbuffer[i + poffset + k] = globalDefineValues[j][k];
					}
					//pbuffer[i + poffset ] = '\n';
					poffset += defnLen - 1;
					break;
				}
			}
			free(defineName);
		}
		else {
			pbuffer[i + poffset] = buffer[i];
		}
	}
	pbuffer[preprocessedFileSize] = '\0';
	fclose(file);
	free(buffer);
	return pbuffer;
}

Shader new_Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath) {
	Shader shader = (Shader)malloc(sizeof(struct Shader));
	if (shader == NULL) return NULL;

	//Open files and read to strings in memory
	const char* vShaderCode = readFile(vertexPath);
	const char* fShaderCode = readFile(fragmentPath);

	//compile the shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		printf("vertex shader compile failed %s\n", infoLog);
		return NULL;
	};
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		printf("fragment shader compile failed %s\n", infoLog);
		return NULL;
	};

	unsigned int geometry = 0;
	const char* gShaderCode = NULL;
	if (geometryPath) {
		gShaderCode = readFile(geometryPath);
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			printf("geometry shader compile failed %s\n", infoLog);
			return NULL;
		};
	}

	//shader program
	shader->id = glCreateProgram();
	glAttachShader(shader->id, vertex);
	if (geometryPath) glAttachShader(shader->id, geometry);
	glAttachShader(shader->id, fragment);
	glLinkProgram(shader->id);
	glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader->id, 512, NULL, infoLog);
		printf("shader program couldn't link %s\n", infoLog);
		return NULL;
	}
	glDeleteShader(vertex);
	if (geometryPath) glDeleteShader(geometry);
	glDeleteShader(fragment);

	if (!shader) printf("shader is null\n");
	free(vShaderCode);
	if (geometryPath) free(gShaderCode);
	free(fShaderCode);
	return shader;
}

void Shader_free(Shader shader) {
	free(shader);
}

void Shader_use(Shader shader) {
	glUseProgram(shader->id);
}

void Shader_setBool(Shader shader, const char* name, int value) {
	Shader_use(shader);
	glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void Shader_setInt(Shader shader, const char* name, int value) {
	Shader_use(shader);
	glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void Shader_setFloat(Shader shader, const char* name, float value) {
	Shader_use(shader);
	glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void Shader_setVec2(Shader shader, const char* name, float x, float y) {
	Shader_use(shader);
	float val[2]; val[0] = x; val[1] = y;
	glUniform2fv(glGetUniformLocation(shader->id, name), 1, val);
}

void Shader_setMat4x4(Shader shader, int location, const GLfloat* value) {
	Shader_use(shader);
	glUniformMatrix4fv(Shader_uniformLocations[location], 1, GL_FALSE, value);
}

/*
void Shader_setMat4x4(Shader shader, const char* name, const GLfloat* value) {
	Shader_use(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, value);
}*/
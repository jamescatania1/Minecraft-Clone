#pragma warning( disable: 6386 )
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shader.h"

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
	fclose(file);
	return buffer;
}

Shader new_Shader(const char* vertexPath, const char* fragmentPath) {
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

	//shader program
	shader->id = glCreateProgram();
	glAttachShader(shader->id, vertex);
	glAttachShader(shader->id, fragment);
	glLinkProgram(shader->id);
	glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader->id, 512, NULL, infoLog);
		printf("shader program couldn't link %s\n", infoLog);
		return NULL;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	if (!shader) printf("shader is null\n");
	free(vShaderCode);
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

void Shader_setMat4x4(Shader shader, int location, const GLfloat* value){
	Shader_use(shader);
	glUniformMatrix4fv(Shader_uniformLocations[location], 1, GL_FALSE, value);
}
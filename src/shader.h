#ifndef _shader_h
#define _shader_h

#include <glad/glad.h>
int Shader_uniformLocations[16];
typedef struct Shader {
	int id;
}
*Shader;

#define UNIFORM_DEFAULT_VERT_TRANSFORM 0
#define UNIFORM_SHADOWMAP_VERT_TRANSFORM 1
#define UNIFORM_SKYBOX_MATRIX 2

//Shader constructor. Use relative paths to vertex/fragment shaders. Leave optional parameters NULL if necessary.
extern Shader new_Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath);

extern void Shader_free(Shader shader);

//Use the shader.
extern void Shader_use(Shader shader);

//Set boolean uniform.
extern void Shader_setBool(Shader shader, const char* name, int value);

//Set integer uniform.
extern void Shader_setInt(Shader shader, const char* name, int value);

//Set float uniform.
extern void Shader_setFloat(Shader shader, const char* name, float value);

//Set vec2 uniform.
extern void Shader_setVec2(Shader shader, const char* name, float x, float y);

//Set mat4 uniform
extern void Shader_setMat4x4(Shader shader, int location, const GLfloat* value);
//extern void Shader_setMat4x4(Shader shader, const char* name, const GLfloat* value);

#endif
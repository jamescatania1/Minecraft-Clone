#ifndef _renderComponent_h
#define _renderComponent_h

#include "shader.h"
#include "math/linearalgebra.h"


typedef struct Transform {
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Mat4x4 matrix;
} *Transform;

typedef struct RenderComponent {
	Shader shader;
	unsigned int texture;
	unsigned int VAO, VBO, EBO;
	int indexCount;
	Transform transform;
	//only updates transform matrix when called manually rather than per update cycle
	int transformStatic;
} *RenderComponent;

extern RenderComponent new_RenderComponent(Shader shader, unsigned int texture, unsigned int VAO, int indexCount, int transformStatic);

extern void RenderComponent_free(RenderComponent renderComponent);

extern void RenderComponent_updatetransformmatrix(RenderComponent renderComponent);

extern Transform new_Transform();

extern Transform new_Transform_at(float x, float y, float z);

extern void Transform_free(Transform transform);
#endif
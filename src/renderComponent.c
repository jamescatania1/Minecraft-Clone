#include <stdlib.h>
#include "renderComponent.h"

RenderComponent new_RenderComponent(Shader shader, unsigned int texture, unsigned int VAO, int indexCount, int transformStatic) {
	RenderComponent this = (RenderComponent)malloc(sizeof(struct RenderComponent));
	if (!this) return NULL;

	this->transform = new_Transform();
	this->shader = shader;
	this->texture = texture;
	this->VAO = VAO;
	this->indexCount = indexCount;
	this->transformStatic = transformStatic;
	RenderComponent_updatetransformmatrix(this);
	return this;
}

void RenderComponent_free(RenderComponent renderComponent) {
	Transform_free(renderComponent->transform);
	free(renderComponent);
}

void RenderComponent_updatetransformmatrix(RenderComponent renderComponent) {
	free(renderComponent->transform->matrix);
	renderComponent->transform->matrix = Mat4x4_Translate(
		renderComponent->transform->position->x,
		renderComponent->transform->position->y, 
		renderComponent->transform->position->z);
}

Transform new_Transform() {
	Transform this = (Transform)malloc(sizeof(struct Transform));
	if (!this) return NULL;
	this->position = new_Vec3(0.0f, 0.0f, 0.0f);
	this->rotation = new_Vec3(0.0f, 0.0f, 0.0f);
	this->scale = new_Vec3(1.0f, 1.0f, 1.0f);
	this->matrix = Mat4x4_Identity();
	return this;
}

Transform new_Transform_at(float x, float y, float z) {
	Transform this = (Transform)malloc(sizeof(struct Transform));
	if (!this) return NULL;
	this->position = new_Vec3(x, y, z);
	this->rotation = new_Vec3(0.0f, 0.0f, 0.0f);
	this->scale = new_Vec3(1.0f, 1.0f, 1.0f);
	this->matrix = Mat4x4_Identity();
	return this;
}

void Transform_free(Transform transform) {
	free(transform->position);
	free(transform->rotation);
	free(transform->scale);
	free(transform->matrix);
	free(transform);
}

#pragma warning(disable: 6011)

#include <stdlib.h>
#include <glfw3.h>
#include <math.h>
#include "main.h"
#include "camera.h"
#include "math/linearalgebra.h"
#include "math/mathutil.h"
#include "input.h"

#define ZFAR_OFFSET 100.0
#define PREFERRED_ASPECT_WIDTH 16
#define PREFERRED_ASPECT_HEIGHT 9
#define ROTATE_INTERP_SPEED 1.1
#define MOVE_INTERP_SPEED 80.0
#define MOVE_SPEED 16.0f

void Camera_init();

void Camera_UpdateMatrix() {
	if (!camera) Camera_init();

	scWidth = fminf(1.0f, (float)windowX / (float)windowY);
	scHeight = fminf(1.0f, (float)windowY / (float)windowX);

	//Camera rotation input
	if (cursorLocked) {
		camera->targRotation->x += mouseSensitivity * resolutionSensitivityOffset * (float)(10.0 * mouseState->dy);
		camera->targRotation->y += mouseSensitivity * resolutionSensitivityOffset * (float)(10.0 * mouseState->dx);
	}
	camera->targRotation->x = fminf(90.0f, fmaxf(-90.0f, camera->targRotation->x));

	camera->rotation->x += ROTATE_INTERP_SPEED * (camera->targRotation->x - camera->rotation->x) * 0.5f;
	camera->rotation->y += ROTATE_INTERP_SPEED * (camera->targRotation->y - camera->rotation->y) * 0.5f;

	while (camera->rotation->y < 0.0f) {
		camera->rotation->y += 360.0f;
		camera->targRotation->y += 360.0f;
	}
	while (camera->rotation->y > 0.0f) {
		camera->rotation->y -= 360.0f;
		camera->targRotation->y -= 360.0f;
	}


	//View Matrix
	Mat4x4 camTr = Mat4x4_Translate(-camera->position->x, -camera->position->y, -camera->position->z);
	Mat4x4 camRotX = Mat4x4_RotateX(camera->targRotation->x);
	Mat4x4 camRotY = Mat4x4_RotateY(camera->targRotation->y - 180.0f);
	Mat4x4 rotMatrix = Mat4x4_Product(camRotY, camRotX);
	Mat4x4 viewMatrix = Mat4x4_Product(camTr, rotMatrix);
	free(camTr); free(camRotX); free(camRotY);
	
	//Projection Matrix
	Mat4x4 projMatrix = new_Mat4x4();
	float aspect = (float)windowX / (float)windowY;
	aspect = 1.0f;
	float t = 1.0f / tanf(camera->fov / 2.0f);
	projMatrix->data[0][0] = t / aspect;
	projMatrix->data[1][1] = t / aspect;
	projMatrix->data[2][2] = (camera->zFar + ZFAR_OFFSET + camera->zNear) / (camera->zNear - camera->zFar - ZFAR_OFFSET);
	projMatrix->data[2][3] = -1.0f;
	projMatrix->data[3][2] = (2.0f * (ZFAR_OFFSET + camera->zFar) * camera->zNear) / (camera->zNear - camera->zFar - ZFAR_OFFSET);

	Mat4x4_free(camera->viewMatrix);
	camera->viewMatrix = viewMatrix;
	Mat4x4_free(camera->projMatrix);
	camera->projMatrix = projMatrix;
	Mat4x4_free(camera->rotProjMatrix);
	camera->rotProjMatrix = Mat4x4_Product(rotMatrix, projMatrix);
	//free(projMatrix);
	free(rotMatrix);


	//calculate forward, right, up vectors
	camera->forward->x = -viewMatrix->data[0][2];
	camera->forward->y = -viewMatrix->data[1][2];
	camera->forward->z = -viewMatrix->data[2][2];
	Vec3_normalize(camera->forward);

	camera->right->x = viewMatrix->data[0][0];
	camera->right->y = viewMatrix->data[1][0];
	camera->right->z = viewMatrix->data[2][0];
	Vec3_normalize(camera->right);

	free(camera->up);
	camera->up = Vec3_cross(camera->right, camera->forward);
	Vec3_normalize(camera->up);


	//Camera movement input
	float horizVel = 0.0f;
	float vertVel = 0.0f;
	if (getKey(KEY_W)->down) vertVel += 1.0f;
	if (getKey(KEY_S)->down) vertVel -= 1.0f;
	if (getKey(KEY_A)->down) horizVel -= 1.0f;
	if (getKey(KEY_D)->down) horizVel += 1.0f;

	float shiftSpeedAdj = getKey(KEY_LEFT_SHIFT)->down ? 2.0f : 1.0f;

	float r_moveVal = horizVel * deltaTime * shiftSpeedAdj * MOVE_SPEED;
	float f_moveVal = vertVel * deltaTime * shiftSpeedAdj * MOVE_SPEED;
	camera->targPosition->x += f_moveVal * camera->forward->x;
	camera->targPosition->y += f_moveVal * camera->forward->y;
	camera->targPosition->z += f_moveVal * camera->forward->z;
	camera->targPosition->x += r_moveVal * camera->right->x;
	camera->targPosition->y += r_moveVal * camera->right->y;
	camera->targPosition->z += r_moveVal * camera->right->z;

	camera->position->x += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->x - camera->position->x) * 0.5f;
	camera->position->y += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->y - camera->position->y) * 0.5f;
	camera->position->z += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->z - camera->position->z) * 0.5f;


	//Update view frustum

	float frusNear = -18.0f;
	float frusFar = -(camera->zFar + ZFAR_OFFSET);
	float vClip = -frusFar * tanf(camera->fov * .475f);
	float hClip = -frusFar * tanf(camera->fov * .6f);
	//printf("%f, %f\n", vClip, hClip);
	Vec3 farFrustum = new_Vec3(frusFar * camera->forward->x, frusFar * camera->forward->y, frusFar * camera->forward->z);

	//near face
	camera->frustum[0]->normal->x = camera->forward->x;
	camera->frustum[0]->normal->y = camera->forward->y;
	camera->frustum[0]->normal->z = camera->forward->z;
	camera->frustum[0]->distance = frusNear;

	//far face
	camera->frustum[1]->normal->x = -camera->forward->x;
	camera->frustum[1]->normal->y = -camera->forward->y;
	camera->frustum[1]->normal->z = -camera->forward->z;
	camera->frustum[1]->distance = frusFar;

	//right face
	Vec3 tmprt = new_Vec3(farFrustum->x - camera->right->x * hClip, farFrustum->y - camera->right->y * hClip, farFrustum->z - camera->right->z * hClip);
	free(camera->frustum[2]->normal);
	camera->frustum[2]->normal = Vec3_cross(tmprt, camera->up);
	free(tmprt);

	//left face
	Vec3 tmplft = new_Vec3(farFrustum->x + camera->right->x * hClip, farFrustum->y + camera->right->y * hClip, farFrustum->z + camera->right->z * hClip);
	free(camera->frustum[3]->normal);
	camera->frustum[3]->normal = Vec3_cross(camera->up, tmplft);
	free(tmplft);

	//top face
	Vec3 tmptp = new_Vec3(farFrustum->x - camera->up->x * vClip, farFrustum->y - camera->up->y * vClip, farFrustum->z - camera->up->z * vClip);
	free(camera->frustum[4]->normal);
	camera->frustum[4]->normal = Vec3_cross(camera->right, tmptp);
	free(tmptp);

	//bottom face
	vClip = -frusFar * tanf(camera->fov * .625f);
	Vec3 tmpbm = new_Vec3(farFrustum->x + camera->up->x * vClip, farFrustum->y + camera->up->y * vClip, farFrustum->z + camera->up->z * vClip);
	free(camera->frustum[5]->normal);
	camera->frustum[5]->normal = Vec3_cross(tmpbm, camera->right);
	free(tmpbm);

	free(farFrustum);
}

void Camera_init() {
	camera = (Camera)malloc(sizeof(struct Camera));
	if (!camera) return NULL;
	camera->viewMatrix = new_Mat4x4();
	camera->projMatrix = new_Mat4x4();
	camera->rotProjMatrix = new_Mat4x4();
	camera->position = new_Vec3(0.0f, 125.0f, 0.0f);
	camera->rotation = new_Vec3(90.0f, 180.0f, 0.0f);
	camera->targPosition = new_Vec3(camera->position->x, camera->position->y, camera->position->z);
	camera->targRotation = new_Vec3(camera->rotation->x, camera->rotation->y, camera->rotation->z);
	camera->forward = new_Vec3(0.0f, 0.0f, 0.0f);
	camera->right = new_Vec3(0.0f, 0.0f, 0.0f);
	camera->up = new_Vec3(0.0f, 0.0f, 0.0f);
	camera->fov = 85.0f * M_PI / 180.0f;
	camera->zFar = 16.0f * RENDER_DISTANCE;
	camera->zNear = 0.1f;
	for (int i = 0; i < 6; i++) {
		camera->frustum[i] = (Camera_FrustumPlane)malloc(sizeof(struct Camera_FrustumPlane));
		camera->frustum[i]->normal = new_Vec3(0.0f, 0.0f, 0.0f);
		camera->frustum[i]->distance = 0.0f;
	}
}

void Camera_free() {
	if (!camera) return;
	free(camera->position);
	free(camera->rotation);
	free(camera->targPosition);
	free(camera->targRotation);
	free(camera->forward);
	free(camera->right);
	free(camera->up);
	Mat4x4_free(camera->viewMatrix);
	Mat4x4_free(camera->projMatrix);
	Mat4x4_free(camera->rotProjMatrix);
	for (int i = 0; i < 6; i++) {
		free(camera->frustum[i]->normal);
		free(camera->frustum[i]);
	}
	free(camera);
}
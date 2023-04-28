#pragma warning(disable: 6011)

#include <stdlib.h>
#include <glfw3.h>
#include <math.h>
#include "main.h"
#include "camera.h"
#include "math/linearalgebra.h"
#include "math/mathutil.h"
#include "input.h"

#define ZFAR_OFFSET 30.0
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
	Mat4x4 camTr = mat4Translate(-camera->position->x, -camera->position->y, camera->position->z);
	Mat4x4 camRotX = mat4RotateX(camera->targRotation->x);
	Mat4x4 camRotY = mat4RotateY(camera->targRotation->y - 180.0f);
	Mat4x4 rotMatrix = mat4Product(camRotY, camRotX);
	Mat4x4 viewMatrix = mat4Product(camTr, rotMatrix);
	free(camTr); free(camRotX); free(camRotY);
	
	//Projection Matrix
	Mat4x4 projMatrix = new_Mat4x4();
	//float aspect = (float)PREFERRED_ASPECT_WIDTH / (float)PREFERRED_ASPECT_HEIGHT;
	float aspect = 1.0f;
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
	camera->rotProjMatrix = mat4Product(rotMatrix, projMatrix);
	//free(projMatrix);
	free(rotMatrix);

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
	camera->targPosition->x -= f_moveVal * viewMatrix->data[0][2];
	camera->targPosition->y -= f_moveVal * viewMatrix->data[1][2];
	camera->targPosition->z += f_moveVal * viewMatrix->data[2][2];
	camera->targPosition->x += r_moveVal * viewMatrix->data[0][0];
	camera->targPosition->y -= r_moveVal * viewMatrix->data[1][0];
	camera->targPosition->z -= r_moveVal * viewMatrix->data[2][0];

	camera->position->x += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->x - camera->position->x) * 0.5f;
	camera->position->y += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->y - camera->position->y) * 0.5f;
	camera->position->z += min(1.0, MOVE_INTERP_SPEED * deltaTime) * (camera->targPosition->z - camera->position->z) * 0.5f;
}

void Camera_init() {
	camera = (Camera)malloc(sizeof(struct Camera));
	if (!camera) return NULL;
	camera->viewMatrix = new_Mat4x4();
	camera->projMatrix = new_Mat4x4();
	camera->rotProjMatrix = new_Mat4x4();
	camera->position = new_Vec3(0.0f, 75.0f, 0.0f);
	camera->rotation = new_Vec3(90.0f, 180.0f, 0.0f);
	camera->targPosition = new_Vec3(camera->position->x, camera->position->y, camera->position->z);
	camera->targRotation = new_Vec3(camera->rotation->x, camera->rotation->y, camera->rotation->z);
	camera->fov = 85.0f * M_PI / 180.0f;
	camera->zFar = 16.0f * RENDER_DISTANCE;
	camera->zNear = 0.1f;
}

void Camera_free() {
	if (!camera) return;
	free(camera->position);
	free(camera->rotation);
	free(camera->targPosition);
	free(camera->targRotation);
	Mat4x4_free(camera->viewMatrix);
	Mat4x4_free(camera->projMatrix);
	Mat4x4_free(camera->rotProjMatrix);
	free(camera);
}
#ifndef _camera_h
#define _camera_h

#include "math/linearalgebra.h"

typedef struct Camera_FrustumPlane {
	Vec3 normal;
	float distance;
} *Camera_FrustumPlane;

typedef struct Camera {
	Mat4x4 viewMatrix;
	Mat4x4 projMatrix;
	Mat4x4 rotProjMatrix;
	Vec3 targPosition;
	Vec3 position;
	Vec3 targRotation;
	Vec3 rotation;
	Vec3 forward, right, up;
	float fov;
	float zFar;
	float zNear;
	Camera_FrustumPlane frustum[6];
} *Camera;
Camera camera;
//In range (0, 1]
float scWidth;
//In range (0, 1]
float scHeight;

extern void Camera_UpdateMatrix();

extern void Camera_free();

#endif
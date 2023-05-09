#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "linearalgebra.h"

Mat4x4 new_Mat4x4() {
	Mat4x4 this = (Mat4x4)malloc(sizeof(struct Mat4x4));
	if (!this) return NULL;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			this->data[i][j] = 0.0f;
		}
	}
	return this;
}
void Mat4x4_free(Mat4x4 matrix) {
	if (matrix) free(matrix);
}

Vec3 new_Vec3(float x, float y, float z) {
	Vec3 this = (Vec3)malloc(sizeof(struct Vec3));
	if (!this) return NULL;
	this->x = x; this->y = y; this->z = z;
	return this;
}
void Vec3_free(Vec3 vector) {
	free(vector);
}

Vec4 new_Vec4(float w, float x, float y, float z) {
	Vec4 this = (Vec4)malloc(sizeof(struct Vec4));
	if (!this) return NULL;
	this->w = w; this->x = x; this->y = y; this->z = z;
	return this;
}
void Vec4_free(Vec4 vector) {
	free(vector);
}


//Vec3 functions:

void Vec3_normalize(Vec3 a) {
	float mag = Vec3_magnitude(a);
	if (mag < .001f) return;
	a->x /= mag; a->y /= mag; a->z /= mag;
}
void Vec3_normalizeTo(Vec3 a, float magnitude) {
	Vec3_normalize(a);
	a->x *= magnitude; a->y *= magnitude; a->z *= magnitude;
}
float Vec3_magnitude(Vec3 a) {
	return sqrtf(a->x * a->x + a->y * a->y + a->z * a->z);
}
Vec3 Vec3_cross(Vec3 a, Vec3 b) {
	Vec3 result = new_Vec3(0.0f, 0.0f, 0.0f);
	result->x = a->y * b->z - a->z * b->y;
	result->y = -a->x * b->z + a->z * b->x;
	result->z = a->x * b->y - b->y * a->x;
	return result;
}
float Vec3_dot(Vec3 a, Vec3 b) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}


//Vec4 functions:

void Vec4_Normalize(Vec4 a) {
	float mag = Vec4_magnitude(a);
	a->w /= mag; a->x /= mag; a->y /= mag; a->z /= mag;
}
float Vec4_magnitude(Vec4 a) {
	return sqrtf(a->w * a->w + a->x * a->x + a->y * a->y + a->z * a->z);
}
Vec4 QuaternionMultiply(Vec4 a, Vec4 b) {
	Vec3 v1 = new_Vec3(a->x, a->y, a->z);
	Vec3 v2 = new_Vec3(b->x, b->y, b->z);
	Vec3 cross = Vec3_cross(v1, v2);
	Vec4 result = new_Vec4(
		a->w * b->w - Vec3_dot(v1, v2),
		a->w * b->x + b->w * a->x + cross->x,
		a->w * b->y + b->w * a->y + cross->y,
		a->w * b->z + b->w * a->z + cross->z);
	free(v1); free(v2); free(cross);
	return result;
}
Vec4 QuaternionInverse(Vec4 a) {
	float normsq = a->x * a->x + a->y * a->y + a->z * a->z + a->w * a->w;
	return new_Vec4(a->w / normsq, -a->x / normsq, -a->y / normsq, -a->z / normsq);
}

Int2 new_Int2(int x, int y) {
	Int2 this = (Int2)malloc(sizeof(struct Int2));
	if (!this) return NULL;
	this->x = x; this->y = y;
	return this;
}
void Int2_free(void* int2) {
	free(int2);
}

//Mat4x4 functions:

Mat4x4 Mat4x4_Identity() {
	Mat4x4 result = new_Mat4x4();
	for (int i = 0; i < 4; i++) result->data[i][i] = 1.0f;
	return result;
}
Vec4 Mat4x4_v4Product(Mat4x4 m, Vec4 v) {
	return new_Vec4(
		m->data[0][0] * v->w + m->data[0][1] * v->x + m->data[0][2] * v->y + m->data[0][3] * v->z,
		m->data[1][0] * v->w + m->data[1][1] * v->x + m->data[1][2] * v->y + m->data[1][3] * v->z,
		m->data[2][0] * v->w + m->data[2][1] * v->x + m->data[2][2] * v->y + m->data[2][3] * v->z,
		m->data[3][0] * v->w + m->data[3][1] * v->x + m->data[3][2] * v->y + m->data[3][3] * v->z
	);
}
Mat4x4 Mat4x4_Product(Mat4x4 a, Mat4x4 b) {
	Mat4x4 result = new_Mat4x4();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result->data[i][j] += a->data[i][k] * b->data[k][j];
			}
		}
	}
	return result;
}
Mat4x4 Mat4x4_Transpose(Mat4x4 m) {
	Mat4x4 result = new_Mat4x4();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result->data[i][j] = m->data[j][i];
		}
	}
	return result;
}
void Mat4x4_Print(Mat4x4 m) {
	printf("\n==================\n %f    %f    %f     %f\n %f    %f    %f     %f\n %f    %f    %f     %f\n %f    %f    %f     %f\n",
		m->data[0][0], m->data[0][1], m->data[0][2], m->data[0][3],
		m->data[1][0], m->data[1][1], m->data[1][2], m->data[1][3],
		m->data[2][0], m->data[2][1], m->data[2][2], m->data[2][3],
		m->data[3][0], m->data[3][1], m->data[3][2], m->data[3][3]);
}
Mat4x4 mat4Scale(float x, float y, float z) {
	Mat4x4 result = new_Mat4x4();
	result->data[0][0] = x;
	result->data[1][1] = y;
	result->data[2][2] = z;
	result->data[3][3] = 1.0f;
	return result;
}
Mat4x4 Mat4x4_Translate(float x, float y, float z) {
	Mat4x4 result = Mat4x4_Identity();
	result->data[3][0] = x;
	result->data[3][1] = y;
	result->data[3][2] = z;
	return result;
}
Mat4x4 Mat4x4_RotateX(float t) {
	t = t * M_PI / 180.f;
	Mat4x4 result = new_Mat4x4();
	result->data[0][0] = 1.0f;
	result->data[3][3] = 1.0f;
	result->data[1][1] = cosf(t);
	result->data[1][2] = sinf(t);
	result->data[2][1] = -sinf(t);
	result->data[2][2] = cosf(t);
	return result;
}
Mat4x4 Mat4x4_RotateY(float t) {
	t = t * M_PI / 180.f;
	Mat4x4 result = new_Mat4x4();
	result->data[1][1] = 1.0f;
	result->data[3][3] = 1.0f;
	result->data[0][0] = cosf(t);
	result->data[0][2] = -sinf(t);
	result->data[2][0] = sinf(t);
	result->data[2][2] = cosf(t);
	return result;
}
Mat4x4 Mat4x4_RotateZ(float t) {
	t = t * M_PI / 180.f;
	Mat4x4 result = new_Mat4x4();
	result->data[2][2] = 1.0f;
	result->data[3][3] = 1.0f;
	result->data[0][0] = cosf(t);
	result->data[0][1] = sinf(t);
	result->data[1][0] = -sinf(t);
	result->data[1][1] = cosf(t);
	return result;
}

Mat4x4 Mat4x4_ViewProjOrthographic(float x, float y, float z, float rotX, float rotY, float near, float far, float size) {
	//View Matrix
	Mat4x4 camTr = Mat4x4_Translate(-x, -y, z);
	Mat4x4 camRotX = Mat4x4_RotateX(rotX);
	Mat4x4 camRotY = Mat4x4_RotateY(rotY - 180.0f);
	Mat4x4 rotMatrix = Mat4x4_Product(camRotY, camRotX);
	Mat4x4 viewMatrix = Mat4x4_Product(camTr, rotMatrix);
	free(rotMatrix);  free(camTr); free(camRotX); free(camRotY);

	//Projection Matrix
	Mat4x4 projMatrix = new_Mat4x4();

	projMatrix->data[0][0] = 1.0f / size;
	projMatrix->data[1][1] = 1.0f / size;
	projMatrix->data[2][2] = -2.0f / (far - near);
	projMatrix->data[3][2] = (-far - near) / (far - near);
	projMatrix->data[3][3] = 1.0f;

	Mat4x4 result = Mat4x4_Product(viewMatrix, projMatrix);
	free(projMatrix);
	free(viewMatrix);
	return result;
}
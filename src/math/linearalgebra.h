#ifndef _linearalgebra_h
#define _linearalgebra_h

#include "glfw3.h"

#define M_PI 3.14159265358979323846 

typedef struct Mat4x4 {
	float data[4][4];
} *Mat4x4;
extern Mat4x4 new_Mat4x4();
extern void Mat4x4_free(Mat4x4 matrix);

typedef struct Vec3 {
	float x, y, z;
} *Vec3;
extern Vec3 new_Vec3(float x, float y, float z);
extern void Vec3_free(Vec3 vector);

typedef struct Vec4 {
	float w, x, y, z;
} *Vec4;
extern Vec4 new_Vec4(float w, float x, float y, float z);
extern void Vec4_free(Vec4 vector);

typedef struct Int2 {
	int x, y;
} *Int2;
extern Int2 new_Int2(int x, int y);
extern void Int2_free(void* int2);

//Vec3 functions:

//Normalizes vector.
extern void Vec3_normalize(Vec3 a);
//Normalizes vector to have norm of magnitude.
extern void Vec3_normalizeTo(Vec3 a, float magnitude);
//Return's vector's norm.
extern float Vec3_magnitude(Vec3 a);
//Returns cross product of two vectors.
extern Vec3 Vec3_cross(Vec3 a, Vec3 b);
//Returns dot product of two vectors.
extern float Vec3_dot(Vec3 a, Vec3 b);


//Vec4 functions:

//Normalizes vector.
extern void Vec4_Normalize(Vec4 a);
//Returns vector's magnitude.
extern float Vec4_magnitude(Vec4 a);
//Quaternion multiplication of a and b.
extern Vec4 QuaternionMultiply(Vec4 a, Vec4 b);
//Quaternion inverse of a.
extern Vec4 QuaternionInverse(Vec4 a);


//Mat4x4 functions:

//Returns identity matrix.
extern Mat4x4 mat4Identity();
//Returns product of matrix * vector, assumes vector is column vector.
extern Vec4 mat4v4Product(Mat4x4 m, Vec4 v);
//Returns matrix multiplication product of a and b.
extern Mat4x4 mat4Product(Mat4x4 a, Mat4x4 b);
//Returns transpose of m.
extern Mat4x4 mat4Transpose(Mat4x4 m);

extern Mat4x4 mat4Scale(float x, float y, float z);

extern Mat4x4 mat4Translate(float x, float y, float z);

extern Mat4x4 mat4RotateX(float t);

extern Mat4x4 mat4RotateY(float t);

extern Mat4x4 mat4RotateZ(float t);

extern void Mat4_print(Mat4x4 m);
#endif
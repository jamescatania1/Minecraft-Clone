#version 430 core

//&GLOBAL
#define SHADOW_CASCADES 3

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[SHADOW_CASCADES];
    float near;
    float far;
};

void main()
{             
	gl_FragDepth = gl_FragCoord.z;
}
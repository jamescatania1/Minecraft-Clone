#version 430 core

//&GLOBAL
#define SHADOW_CASCADES 3

flat in int cascade[];

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[SHADOW_CASCADES];
    float near;
    float far;
};

void main() {
    int k = -1;
    for(int i = 0; i < SHADOW_CASCADES; i++) {
        if(cascade[i] > k) k = cascade[i];
    }
    if(cascade[k] == -1) return;
    for (int i = 0; i < SHADOW_CASCADES; i++) {
        gl_Position = sunMatrix[k] * gl_in[i].gl_Position;
        gl_Layer = k;
        EmitVertex();
    }
    EndPrimitive();
}
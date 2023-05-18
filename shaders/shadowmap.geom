#version 430 core

flat in int cascade[];

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[3];
    float near;
    float far;
};

void main() {

    if(cascade[0] == -1) return;
    for (int i = 0; i < 3; i++) {
        gl_Position = sunMatrix[cascade[0]] * gl_in[i].gl_Position;
        gl_Layer = cascade[0];
        EmitVertex();
    }
    EndPrimitive();
}